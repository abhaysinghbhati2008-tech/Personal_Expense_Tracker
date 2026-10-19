#include"Expenses.h"
#include<string>
#include<iostream>
#include<vector>
#include<limits>
using namespace std;
vector<Expense> Exp;
    int id = 0;
        void Catch(){
            cout<<"Invaild Input enter again";
         cin.clear();
        // numeric_limits<streamsize>::max() used to clear the all the input the user enters
        cin.ignore(numeric_limits<streamsize>::max() , '\n');
        }

void AddExpense(){
    id++;

    //Adding expense with the error resolving when user enters something wrong 
    while(true){
        double Amount;
        cout<<"Enter the Amount";
        if(cin >> Amount && Amount > 0) break;
        //  cout<<"Invaild Input enter again";
        //  cin.clear();
        // // numeric_limits<streamsize>::max() used to clear the all the input the user enters
        // cin.ignore(numeric_limits<streamsize>::max() , '\n');
        Catch();

    }
    // make it in a separete function for all three inputs
    while(true){
        string name;
        cout<<"Enter the name";
        if(cin>> name) break;
        Catch();
    }
    while(true){
        string Discription;
        cout<<"enter the Discription";
        
    }
    
}

