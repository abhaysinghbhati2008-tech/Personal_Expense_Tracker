#include"Expenses.h"
#include<string>
#include<iostream>
#include<vector>
#include<limits>
#include"Date.h"
using namespace std;
vector<Expense> Exp;
    int id = 0;
        void Catch(){
            cout<<"Invaild Input enter again"<<endl;
         cin.clear();
        // numeric_limits<streamsize>::max() used to clear the all the input the user enters
        cin.ignore(numeric_limits<streamsize>::max() , '\n');
        }

void AddExpense(){
            double Amount;
               string name ,discription , date;
               bool isfixed;
    //Adding expense with the error resolving when user enters something wrong 
     while(true){
     
        cout<<"Enter the name :";
        if(getline(cin , name) && !name.empty()) break;
        Catch();
    }
    while(true){

        cout<<"Enter the Amount :";
        if(cin >> Amount && Amount > 0){
            cin.ignore(numeric_limits<streamsize>::max() , '\n');
            break;
        }
        //  cout<<"Invaild Input enter again";
        //  cin.clear();
        // // numeric_limits<streamsize>::max() used to clear the all the input the user enters
        // cin.ignore(numeric_limits<streamsize>::max() , '\n');
        Catch();

    }
    // make it in a separete function for all three inputs
    
    //  for the date input but before that i have to make a vaildation function to check whether 
    //  the string enters is correct or not
    while(true){
        cout<<"Enter the Date (YYYY-MM-DD) : ";
        cin>>date;
        if(IsValid(date)) break;
        cout<<"Invaild Input Try again";
    }
    while(true){
     
        cout<<"enter the Discription :";
        if(getline(cin , discription) && !discription.empty()) break;
        Catch();
    }
    // isfixed
    while(true){
        string choice;
        cout<<"Is this a Fixed Expense (yes/no) ";
        cin >> choice;
        if(choice == "yes" || choice == "Yes" || choice == "YES"){
            isfixed = true;
            break;
        }
        else if(choice == "no" || choice == "No" || choice == "NO"){
            isfixed = false;
            break;
        }
        Catch();
    }
    string frequency = "none";
    if(isfixed){
        while(true){
            cout<<"enter the frequency Monthly / Weekly / Daily : ";
            cin>>frequency;
            if(frequency == "Monthly"
            || frequency == "Weekly"
            || frequency == "Daily") break;
            Catch();
        }
    }
    // adding it in vector
    Expense e;
    e.amount = Amount;
    e.name = name;
    e.id = ++id;
    e.IsFixed = isfixed;
    e.frequency = frequency;
    e.Date = date;
     
    Exp.push_back(e);
    cout<<"Expense added Successfully \n";
     cout<<"Name = "<<e.name<<endl;
    cout<<"Amount = "<<e.amount<<endl;
    cout<<"ID = "<<e.id<<endl;
    cout<<"Date ="<<e.Date<<endl;
    cout<<"isFixed = ";
    if(isfixed){
         cout<<"YES"<<endl;
         cout<<"Frequncy = "<<e.frequency;
    }
    else cout<<"NO";
    cout<<endl;


}



