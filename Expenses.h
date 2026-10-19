#pragma once
#include<string>
class Expense{
    public:
    int id;
    std::string Date; //date 
    std::string name; // name catergory or some kind of discription
    double amount;  //amount 
    bool IsFixed; // check if this is fixed or not
    std::string frequency; //like weekly monthly or daily
    std::string Discritpion;
};
