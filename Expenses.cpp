#include "Expenses.h"

#include "Date.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace {
const char* kCsvHeader = "id,date,name,amount,is_fixed,frequency,description";

std::string EscapeCsv(const std::string& value) {
    std::string escaped = "\"";
    for (char character : value) {
        if (character == '"') {
            escaped += "\"\"";
        } else {
            escaped += character;
        }
    }
    escaped += '"';
    return escaped;
}

std::vector<std::string> ParseCsvRow(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    bool quoteClosed = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char character = line[i];
        if (inQuotes) {
            if (character == '"' && i + 1 < line.size() &&
                line[i + 1] == '"') {
                field += '"';
                ++i;
            } else if (character == '"') {
                inQuotes = false;
                quoteClosed = true;
            } else {
                field += character;
            }
        } else if (character == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
            quoteClosed = false;
        } else if (character == '"' && field.empty() && !quoteClosed) {
            inQuotes = true;
        } else if (quoteClosed) {
            throw std::runtime_error("Unexpected text after a quoted CSV field");
        } else {
            field += character;
        }
    }

    if (inQuotes) {
        throw std::runtime_error("Unterminated quoted CSV field");
    }
    fields.push_back(field);
    return fields;
}

int ParseId(const std::string& value) {
    std::size_t parsed = 0;
    const int id = std::stoi(value, &parsed);
    if (parsed != value.size()) {
        throw std::runtime_error("Invalid expense ID");
    }
    return id;
}

double ParseAmount(const std::string& value) {
    std::size_t parsed = 0;
    const double amount = std::stod(value, &parsed);
    if (parsed != value.size() || !std::isfinite(amount)) {
        throw std::runtime_error("Invalid expense amount");
    }
    return amount;
}

bool ContainsLineBreak(const std::string& value) {
    return value.find_first_of("\r\n") != std::string::npos;
}
}  // namespace

std::vector<Expense> LoadExpenses(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return {};
    }

    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open expense file: " + path);
    }

    std::string line;
    if (!std::getline(input, line)) {
        throw std::runtime_error("Expense file has an invalid or missing header");
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    if (line != kCsvHeader) {
        throw std::runtime_error("Expense file has an invalid or missing header");
    }

    std::vector<Expense> expenses;
    std::unordered_set<int> ids;
    std::size_t lineNumber = 1;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        try {
            const std::vector<std::string> fields = ParseCsvRow(line);
            if (fields.size() != 7) {
                throw std::runtime_error("Expected seven fields");
            }

            Expense expense;
            expense.id = ParseId(fields[0]);
            expense.date = fields[1];
            expense.name = fields[2];
            expense.amount = ParseAmount(fields[3]);
            if (fields[4] != "0" && fields[4] != "1") {
                throw std::runtime_error("Invalid fixed-expense flag");
            }
            expense.isFixed = fields[4] == "1";
            expense.frequency = fields[5];
            expense.description = fields[6];

            std::string error;
            if (!ValidateExpense(expense, error)) {
                throw std::runtime_error(error);
            }
            if (!ids.insert(expense.id).second) {
                throw std::runtime_error("Duplicate expense ID");
            }
            expenses.push_back(std::move(expense));
        } catch (const std::exception& exception) {
            throw std::runtime_error("Invalid expense file at line " +
                                     std::to_string(lineNumber) + ": " +
                                     exception.what());
        }
    }
    if (input.bad()) {
        throw std::runtime_error("Could not read expense file: " + path);
    }
    return expenses;
}

void SaveExpenses(const std::string& path,
                  const std::vector<Expense>& expenses) {
    const std::string temporaryPath = path + ".tmp";
    std::unordered_set<int> ids;
    for (const Expense& expense : expenses) {
        std::string error;
        if (!ValidateExpense(expense, error)) {
            throw std::invalid_argument("Cannot save expense: " + error);
        }
        if (!ids.insert(expense.id).second) {
            throw std::invalid_argument("Cannot save duplicate expense IDs");
        }
    }

    {
        std::ofstream output(temporaryPath, std::ios::trunc);
        if (!output) {
            throw std::runtime_error("Could not write expense file: " + path);
        }

        output << kCsvHeader << '\n' << std::setprecision(
            std::numeric_limits<double>::max_digits10);
        for (const Expense& expense : expenses) {
            output << expense.id << ',' << EscapeCsv(expense.date) << ','
                   << EscapeCsv(expense.name) << ',' << expense.amount << ','
                   << (expense.isFixed ? 1 : 0) << ','
                   << EscapeCsv(expense.frequency) << ','
                   << EscapeCsv(expense.description) << '\n';
        }
        output.flush();
        if (!output) {
            throw std::runtime_error("Could not finish writing expense file: " +
                                     path);
        }
    }

    std::error_code error;
    std::filesystem::rename(temporaryPath, path, error);
    if (error) {
        std::error_code cleanupError;
        std::filesystem::remove(temporaryPath, cleanupError);
        throw std::runtime_error("Could not replace expense file: " +
                                 error.message());
    }
}

bool ValidateExpense(const Expense& expense, std::string& error) {
    if (expense.id <= 0) {
        error = "Expense ID must be positive";
        return false;
    }
    if (!IsValid(expense.date)) {
        error = "Date must be a valid YYYY-MM-DD date";
        return false;
    }
    if (expense.name.empty() || ContainsLineBreak(expense.name)) {
        error = "Name is required and cannot contain line breaks";
        return false;
    }
    if (!std::isfinite(expense.amount) || expense.amount <= 0) {
        error = "Amount must be a positive number";
        return false;
    }
    if (ContainsLineBreak(expense.frequency) ||
        ContainsLineBreak(expense.description)) {
        error = "Expense fields cannot contain line breaks";
        return false;
    }
    if (expense.isFixed) {
        if (expense.frequency != "Daily" && expense.frequency != "Weekly" &&
            expense.frequency != "Monthly") {
            error = "Fixed expenses must use Daily, Weekly, or Monthly frequency";
            return false;
        }
    } else if (!expense.frequency.empty() && expense.frequency != "none") {
        error = "Non-fixed expenses cannot have a recurrence frequency";
        return false;
    }
    return true;
}

bool AddExpense(std::vector<Expense>& expenses, const Expense& expense,
                std::string& error) {
    if (!ValidateExpense(expense, error)) {
        return false;
    }
    for (const Expense& existing : expenses) {
        if (existing.id == expense.id) {
            error = "An expense with this ID already exists";
            return false;
        }
    }
    expenses.push_back(expense);
    return true;
}

bool DeleteExpense(std::vector<Expense>& expenses, int id) {
    for (auto expense = expenses.begin(); expense != expenses.end(); ++expense) {
        if (expense->id == id) {
            expenses.erase(expense);
            return true;
        }
    }
    return false;
}

std::vector<Expense> FilterExpenses(const std::vector<Expense>& expenses,
                                    const ExpenseFilter& filter) {
    if (filter.startDate && !IsValid(*filter.startDate)) {
        throw std::invalid_argument("Start date must be a valid YYYY-MM-DD date");
    }
    if (filter.endDate && !IsValid(*filter.endDate)) {
        throw std::invalid_argument("End date must be a valid YYYY-MM-DD date");
    }
    if (filter.startDate && filter.endDate &&
        *filter.endDate < *filter.startDate) {
        throw std::invalid_argument("End date must not be before start date");
    }
    if (filter.minimumAmount &&
        (!std::isfinite(*filter.minimumAmount) ||
         *filter.minimumAmount < 0)) {
        throw std::invalid_argument("Minimum amount must be zero or greater");
    }
    if (filter.maximumAmount &&
        (!std::isfinite(*filter.maximumAmount) ||
         *filter.maximumAmount < 0)) {
        throw std::invalid_argument("Maximum amount must be zero or greater");
    }
    if (filter.minimumAmount && filter.maximumAmount &&
        *filter.maximumAmount < *filter.minimumAmount) {
        throw std::invalid_argument(
            "Maximum amount must not be below minimum amount");
    }

    std::vector<Expense> matches;
    for (const Expense& expense : expenses) {
        if (filter.startDate && expense.date < *filter.startDate) {
            continue;
        }
        if (filter.endDate && expense.date > *filter.endDate) {
            continue;
        }
        if (filter.minimumAmount &&
            expense.amount < *filter.minimumAmount) {
            continue;
        }
        if (filter.maximumAmount &&
            expense.amount > *filter.maximumAmount) {
            continue;
        }
        matches.push_back(expense);
    }
    return matches;
}

double TotalExpenses(const std::vector<Expense>& expenses) {
    double total = 0;
    for (const Expense& expense : expenses) {
        total += expense.amount;
    }
    return total;
}
