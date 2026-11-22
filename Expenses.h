#pragma once

#include <optional>
#include <string>
#include <vector>

struct Expense {
    int id = 0;
    std::string date;
    std::string name;
    double amount = 0.0;
    bool isFixed = false;
    std::string frequency;
    std::string description;
};

struct ExpenseFilter {
    std::optional<std::string> startDate;
    std::optional<std::string> endDate;
    std::optional<double> minimumAmount;
    std::optional<double> maximumAmount;
};

std::vector<Expense> LoadExpenses(const std::string& path);
void SaveExpenses(const std::string& path,
                  const std::vector<Expense>& expenses);
bool ValidateExpense(const Expense& expense, std::string& error);
bool AddExpense(std::vector<Expense>& expenses, const Expense& expense,
                std::string& error);
bool DeleteExpense(std::vector<Expense>& expenses, int id);
std::vector<Expense> FilterExpenses(const std::vector<Expense>& expenses,
                                    const ExpenseFilter& filter);
double TotalExpenses(const std::vector<Expense>& expenses);
