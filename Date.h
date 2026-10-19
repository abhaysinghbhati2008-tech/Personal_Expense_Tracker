#include<iostream>
using namespace std;
#include<string>
#include<sstream>
#include<cctype>
struct Date{
    public:
    int day;
    int month;
    int year;
};

// vaildation function for a given string 
bool IsValid(const string& date){
    if(date.size() != 10) return false;// checks that is it in the formate of YYYY-MM-DD
    // if(date[4] != '-' || date[7] != '-') return; //checks for the dashes this can checked by the stringstream
    // but by this we dont hvae to rely on the prase string 
    if(date[4] != '-' || date[7] != '-') return false;
    // this loop checks for teh last word like 2026-10-1a so it checks for the a 
    for(int i =0 ;i<date.size();i++){
        if(i == 4 || i == 7) continue;
        // if(!isdigit(date[i])) return false; 
        /*
        beacuse isdigit function works by looking the asii valuse and its made for only +ve interger 
        but when it get -ve it get confused
        eg when user enters this    2024-05-1€
        € -- this get mroe them 127 and them a signed char treat it like a -ve number. and that can 
        cause this trouble 

        to solve this we use ungined char 
        */
       if(!isdigit(static_cast<unsigned char>(date[i]))) return false;
    }
    int year , day, month;
    char dash1 , dash2;
    stringstream ss(date);
    ss >>year>>dash1>>month>>dash2>>day;
    if(ss.fail() || dash1 != '-' || dash2 != '-') return false;
    if(year < 2000 || year > 2100) return false;
    if(month < 1 || month > 12) return false;
    if(day < 1 || day > 31) return false;

    // cehcking for the leap year
    int DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if((year%4 == 0 && year%100 != 0) || year%400 == 0){
        DaysInMonth[1] = 29;
    }
    if(day > DaysInMonth[month -1]) return false;
     
    return true;
}

Date stringtodate(const string& date) {
    Date ans;
    if (!IsValid(date)) {
        return ans; // returns zero
    }
    int year, month, day;
    char dash1, dash2;

    stringstream ss(date);

    ss >> year >> dash1 >> month >> dash2 >> day;

    ans.day = day;
    ans.year = year;
    ans.month = month;

    return ans;
}
// date  -- > string to write in the file
string datetostring(const Date& date){
    string year = to_string(date.year);
    string month = ((date.month >= 10) ? "" : "0") + to_string(date.month);
    string day = ((date.day >= 10) ? "" : "0") + to_string(date.day);
    return year +"-"+month+"-"+day;
}

// string to date.
bool IsSame(const Date& d1 , const Date& d2){
    return (d1.day == d2.day && d1.month == d2.month && d1.year == d2.year);
}

// check for isbefore 
bool isBefore(const Date& d1, const Date& d2) {
    if (d1.year != d2.year)
        return d1.year < d2.year;

    if (d1.month != d2.month)
        return d1.month < d2.month;

    return d1.day < d2.day;
}

//check for is after 
bool isAfter(const Date& d1, const Date& d2) {
    return !isBefore(d1, d2) && !IsSame(d1, d2);
}
