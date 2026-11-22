#pragma once

#include <string>

struct Date {
    int day = 0;
    int month = 0;
    int year = 0;
};

inline bool IsValid(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }

    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i != 4 && i != 7 &&
            (date[i] < '0' || date[i] > '9')) {
            return false;
        }
    }

    const int year = std::stoi(date.substr(0, 4));
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    if (year < 1 || month < 1 || month > 12) {
        return false;
    }

    const int daysInMonth[] = {31, 28, 31, 30, 31, 30,
                               31, 31, 30, 31, 30, 31};
    int maximumDay = daysInMonth[month - 1];
    if (month == 2 &&
        ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
        maximumDay = 29;
    }
    return day >= 1 && day <= maximumDay;
}

inline Date stringtodate(const std::string& date) {
    if (!IsValid(date)) {
        return {};
    }
    return {std::stoi(date.substr(8, 2)), std::stoi(date.substr(5, 2)),
            std::stoi(date.substr(0, 4))};
}

inline std::string datetostring(const Date& date) {
    std::string year = std::to_string(date.year);
    std::string month = date.month < 10 ? "0" + std::to_string(date.month)
                                        : std::to_string(date.month);
    std::string day = date.day < 10 ? "0" + std::to_string(date.day)
                                    : std::to_string(date.day);
    if (year.size() < 4) {
        year.insert(0, 4 - year.size(), '0');
    }
    return year + "-" + month + "-" + day;
}

inline bool IsSame(const Date& first, const Date& second) {
    return first.day == second.day && first.month == second.month &&
           first.year == second.year;
}

inline bool isBefore(const Date& first, const Date& second) {
    if (first.year != second.year) {
        return first.year < second.year;
    }
    if (first.month != second.month) {
        return first.month < second.month;
    }
    return first.day < second.day;
}

inline bool isAfter(const Date& first, const Date& second) {
    return !isBefore(first, second) && !IsSame(first, second);
}
