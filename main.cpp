#include "Expenses.h"

#include "Date.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
bool ReadLine(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, value));
}

std::string Lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                   });
    return value;
}

bool ParseAmount(const std::string& text, double& amount) {
    try {
        std::size_t parsed = 0;
        amount = std::stod(text, &parsed);
        return parsed == text.size() && std::isfinite(amount) && amount >= 0;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool ReadAmount(const std::string& prompt, double& amount) {
    std::string input;
    while (ReadLine(prompt, input)) {
        if (ParseAmount(input, amount) && amount > 0) {
            return true;
        }
        std::cout << "Enter a valid amount greater than zero.\n";
    }
    return false;
}

bool ReadOptionalAmount(const std::string& prompt,
                        std::optional<double>& amount) {
    std::string input;
    while (ReadLine(prompt, input)) {
        if (input.empty()) {
            amount.reset();
            return true;
        }
        double parsedAmount = 0;
        if (ParseAmount(input, parsedAmount)) {
            amount = parsedAmount;
            return true;
        }
        std::cout << "Enter a valid amount zero or greater, or leave blank.\n";
    }
    return false;
}

bool ReadOptionalDate(const std::string& prompt,
                      std::optional<std::string>& date) {
    std::string input;
    while (ReadLine(prompt, input)) {
        if (input.empty()) {
            date.reset();
            return true;
        }
        if (IsValid(input)) {
            date = input;
            return true;
        }
        std::cout << "Enter a valid date in YYYY-MM-DD format, or leave blank.\n";
    }
    return false;
}

void PrintExpenses(const std::vector<Expense>& expenses) {
    if (expenses.empty()) {
        std::cout << "No expenses match those filters.\n";
        return;
    }

    std::cout << std::left << std::setw(6) << "ID" << std::setw(13) << "Date"
              << std::setw(24) << "Name" << std::right << std::setw(12)
              << "Amount" << "  Type / Frequency\n";
    std::cout << std::string(76, '-') << '\n';
    for (const Expense& expense : expenses) {
        std::cout << std::left << std::setw(6) << expense.id
                  << std::setw(13) << expense.date << std::setw(24)
                  << expense.name << std::right << std::fixed
                  << std::setprecision(2) << std::setw(12) << expense.amount
                  << "  " << (expense.isFixed
                                  ? "Fixed (" + expense.frequency + ")"
                                  : "One-time")
                  << '\n';
        if (!expense.description.empty()) {
            std::cout << "      " << expense.description << '\n';
        }
    }
}

bool AddFromPrompt(std::vector<Expense>& expenses,
                   const std::string& dataPath) {
    Expense expense;
    if (expenses.size() >=
        static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        std::cout << "Cannot add more expenses: the ID limit was reached.\n";
        return true;
    }
    int largestId = 0;
    for (const Expense& existing : expenses) {
        largestId = std::max(largestId, existing.id);
    }
    if (largestId == std::numeric_limits<int>::max()) {
        std::cout << "Cannot add more expenses: the ID limit was reached.\n";
        return true;
    }
    expense.id = largestId + 1;

    if (!ReadLine("Name: ", expense.name)) {
        return false;
    }
    while (expense.name.empty()) {
        std::cout << "Name cannot be empty.\n";
        if (!ReadLine("Name: ", expense.name)) {
            return false;
        }
    }
    if (!ReadAmount("Amount: ", expense.amount) ||
        !ReadLine("Date (YYYY-MM-DD): ", expense.date)) {
        return false;
    }
    while (!IsValid(expense.date)) {
        std::cout << "Enter a valid date in YYYY-MM-DD format.\n";
        if (!ReadLine("Date (YYYY-MM-DD): ", expense.date)) {
            return false;
        }
    }
    if (!ReadLine("Description (optional): ", expense.description)) {
        return false;
    }

    std::string fixedChoice;
    if (!ReadLine("Fixed recurring expense? (yes/no): ", fixedChoice)) {
        return false;
    }
    fixedChoice = Lowercase(fixedChoice);
    while (fixedChoice != "yes" && fixedChoice != "no") {
        std::cout << "Enter yes or no.\n";
        if (!ReadLine("Fixed recurring expense? (yes/no): ", fixedChoice)) {
            return false;
        }
        fixedChoice = Lowercase(fixedChoice);
    }
    expense.isFixed = fixedChoice == "yes";
    if (expense.isFixed) {
        if (!ReadLine("Frequency (daily/weekly/monthly): ",
                      expense.frequency)) {
            return false;
        }
        expense.frequency = Lowercase(expense.frequency);
        while (expense.frequency != "daily" && expense.frequency != "weekly" &&
               expense.frequency != "monthly") {
            std::cout << "Choose daily, weekly, or monthly.\n";
            if (!ReadLine("Frequency (daily/weekly/monthly): ",
                          expense.frequency)) {
                return false;
            }
            expense.frequency = Lowercase(expense.frequency);
        }
        expense.frequency[0] = static_cast<char>(
            std::toupper(static_cast<unsigned char>(expense.frequency[0])));
    }

    std::vector<Expense> updatedExpenses = expenses;
    std::string error;
    if (!AddExpense(updatedExpenses, expense, error)) {
        std::cout << "Could not add expense: " << error << '\n';
        return true;
    }
    SaveExpenses(dataPath, updatedExpenses);
    expenses = std::move(updatedExpenses);
    std::cout << "Expense #" << expense.id << " saved.\n";
    return true;
}

bool SearchFromPrompt(const std::vector<Expense>& expenses) {
    ExpenseFilter filter;
    if (!ReadOptionalDate("From date (YYYY-MM-DD, blank for any): ",
                          filter.startDate) ||
        !ReadOptionalDate("To date (YYYY-MM-DD, blank for any): ",
                          filter.endDate) ||
        !ReadOptionalAmount("Minimum amount (blank for any): ",
                            filter.minimumAmount) ||
        !ReadOptionalAmount("Maximum amount (blank for any): ",
                            filter.maximumAmount)) {
        return false;
    }

    try {
        const std::vector<Expense> results = FilterExpenses(expenses, filter);
        PrintExpenses(results);
        std::cout << "Total for matching expenses: " << std::fixed
                  << std::setprecision(2) << TotalExpenses(results) << '\n';
    } catch (const std::invalid_argument& error) {
        std::cout << "Invalid range: " << error.what() << '\n';
    }
    return true;
}

bool DeleteFromPrompt(std::vector<Expense>& expenses,
                      const std::string& dataPath) {
    std::string input;
    if (!ReadLine("Expense ID to delete: ", input)) {
        return false;
    }

    int id = 0;
    try {
        std::size_t parsed = 0;
        id = std::stoi(input, &parsed);
        if (parsed != input.size() || id <= 0) {
            throw std::invalid_argument("invalid ID");
        }
    } catch (const std::exception&) {
        std::cout << "Enter a valid positive expense ID.\n";
        return true;
    }

    auto found = std::find_if(expenses.begin(), expenses.end(),
                              [id](const Expense& expense) {
                                  return expense.id == id;
                              });
    if (found == expenses.end()) {
        std::cout << "No expense found with ID " << id << ".\n";
        return true;
    }
    std::string confirmation;
    if (!ReadLine("Delete this expense permanently? (yes/no): ",
                  confirmation)) {
        return false;
    }
    if (Lowercase(confirmation) != "yes") {
        std::cout << "Deletion cancelled.\n";
        return true;
    }

    std::vector<Expense> updatedExpenses = expenses;
    DeleteExpense(updatedExpenses, id);
    SaveExpenses(dataPath, updatedExpenses);
    expenses = std::move(updatedExpenses);
    std::cout << "Expense #" << id << " deleted.\n";
    return true;
}
}  // namespace

int main(int argc, char* argv[]) {
    std::string dataPath = "expenses.csv";
    if (argc == 3 && std::string(argv[1]) == "--data") {
        dataPath = argv[2];
    } else if (argc != 1) {
        std::cerr << "Usage: " << argv[0] << " [--data FILE]\n";
        return 2;
    }

    try {
        std::vector<Expense> expenses = LoadExpenses(dataPath);
        std::cout << "Loaded " << expenses.size() << " expense(s) from "
                  << dataPath << ".\n";

        while (true) {
            std::cout << "\nPersonal Expense Tracker\n"
                      << "1. Add expense\n"
                      << "2. Search / view expenses and total\n"
                      << "3. Delete expense\n"
                      << "4. Exit\n";
            std::string choice;
            if (!ReadLine("Choose an option: ", choice)) {
                break;
            }

            if (choice == "1") {
                if (!AddFromPrompt(expenses, dataPath)) {
                    break;
                }
            } else if (choice == "2") {
                if (!SearchFromPrompt(expenses)) {
                    break;
                }
            } else if (choice == "3") {
                if (!DeleteFromPrompt(expenses, dataPath)) {
                    break;
                }
            } else if (choice == "4") {
                break;
            } else {
                std::cout << "Choose 1, 2, 3, or 4.\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Expense tracker error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
