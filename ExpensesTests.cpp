#include "Date.h"
#include "Expenses.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
Expense MakeExpense(int id, std::string date, double amount,
                    std::string name = "expense") {
    Expense expense;
    expense.id = id;
    expense.date = std::move(date);
    expense.name = std::move(name);
    expense.amount = amount;
    return expense;
}

void TestDates() {
    assert(IsValid("2024-02-29"));
    assert(!IsValid("2023-02-29"));
    assert(!IsValid("2024-13-01"));
    assert(!IsValid("2024-1-01"));
    assert(datetostring(stringtodate("2024-02-29")) == "2024-02-29");
}

void TestFilteringAndTotals() {
    std::vector<Expense> expenses = {
        MakeExpense(1, "2024-02-28", 99.99),
        MakeExpense(2, "2024-02-29", 100.00),
        MakeExpense(3, "2024-03-01", 100.00),
        MakeExpense(4, "2024-03-02", 200.00),
    };

    ExpenseFilter filter;
    filter.startDate = "2024-02-29";
    filter.endDate = "2024-03-01";
    filter.minimumAmount = 100.00;
    filter.maximumAmount = 100.00;
    const std::vector<Expense> matches = FilterExpenses(expenses, filter);
    assert(matches.size() == 2);
    assert(TotalExpenses(matches) == 200.00);

    ExpenseFilter noMatches;
    noMatches.startDate = "2025-01-01";
    assert(FilterExpenses(expenses, noMatches).empty());
    assert(TotalExpenses(FilterExpenses(expenses, noMatches)) == 0.0);

    ExpenseFilter reversedDates;
    reversedDates.startDate = "2024-03-01";
    reversedDates.endDate = "2024-02-29";
    bool rejected = false;
    try {
        FilterExpenses(expenses, reversedDates);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);

    ExpenseFilter reversedAmounts;
    reversedAmounts.minimumAmount = 10;
    reversedAmounts.maximumAmount = 5;
    rejected = false;
    try {
        FilterExpenses(expenses, reversedAmounts);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);
}

void TestPersistenceAndDeletion() {
    const auto unique = std::chrono::steady_clock::now()
                            .time_since_epoch()
                            .count();
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        ("expense_tracker_test_" + std::to_string(unique) + ".csv");

    Expense fixed = MakeExpense(1, "2024-03-01", 45.5, "Rent, \"shared\"");
    fixed.isFixed = true;
    fixed.frequency = "Monthly";
    fixed.description = "First floor, near \"the park\"";
    std::vector<Expense> expenses{fixed};

    SaveExpenses(path.string(), expenses);
    const std::vector<Expense> loaded = LoadExpenses(path.string());
    assert(loaded.size() == 1);
    assert(loaded[0].name == fixed.name);
    assert(loaded[0].description == fixed.description);
    assert(loaded[0].isFixed);
    assert(loaded[0].frequency == "Monthly");
    assert(loaded[0].amount == fixed.amount);

    std::string error;
    Expense invalid = MakeExpense(2, "2024-02-30", 5);
    assert(!AddExpense(expenses, invalid, error));
    Expense duplicate = fixed;
    assert(!AddExpense(expenses, duplicate, error));
    Expense added = MakeExpense(2, "2024-03-02", 5);
    assert(AddExpense(expenses, added, error));
    assert(DeleteExpense(expenses, 1));
    assert(!DeleteExpense(expenses, 1));
    assert(expenses.size() == 1 && expenses[0].id == 2);

    std::filesystem::remove(path);
}
}  // namespace

int main() {
    TestDates();
    TestFilteringAndTotals();
    TestPersistenceAndDeletion();
    std::cout << "All expense tracker tests passed.\n";
}
