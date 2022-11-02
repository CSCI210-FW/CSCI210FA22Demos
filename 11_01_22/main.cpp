#include <iostream>
#include <string>
#include <climits>
#include "sqlite3.h"

using namespace std;

int callback(void*, int, char**, char**);
void viewAssignmentsByProject(sqlite3 *);

int main()
{
    sqlite3 * db;
    int rc;

    rc  = sqlite3_open_v2("ConstructCo.db", &db, SQLITE_OPEN_READWRITE, NULL);
    if(rc != SQLITE_OK)
    {
        cout << "Error opening database: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 0;
    }
    else
    {
        cout << "Database opened successfully." << endl;
    }

    string query = "select emp_num as 'Employee Number', emp_fname || ' ' || emp_lname as 'Employee Name' from employee";
    char * err;
    rc = sqlite3_exec(db, query.c_str(), callback, NULL, &err);
    if(rc != SQLITE_OK)
    {
        cout << "There was an error - select callback: " << err << endl;
        sqlite3_free(err);
    }

    string lname, fname, hiredate, mi;
    int job, years;
    lname = "Brown";
    fname = "Charlie";
    hiredate = "2022-11-01";
    job = 504;
    years = 0;

    query = "delete from employee where emp_lname = 'Brown' and emp_fname = 'Charlie'";
    rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
    if(rc != SQLITE_OK)
    {
        cout << "There was an error - delete: " << err << endl;
        sqlite3_free(err);
    }

    query = "insert into employee(emp_lname, emp_fname, emp_hiredate, job_code, emp_years) ";
    query += "values ('";
    query += lname + "', '";
    query += fname + "' , '";
    query += hiredate + "', ";
    query += to_string(job) + ", ";
    query += to_string(years) + ");";
    //cout << query << endl;

    rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
    if(rc != SQLITE_OK)
    {
        cout << "There was an error - insert: " << err << endl;
        sqlite3_free(err);
    }
    else
    {
        int emp_num = sqlite3_last_insert_rowid(db);
        cout << fname << " " << lname << " inserted into the database as employee number " << emp_num << endl;
    }
    int choice;
    cout << "Would you like to see assignments by:" << endl;
    cout << "1. Project" << endl;
    cout << "2. Employee" << endl;
    cin >> choice;
    while(!cin || choice != 1)
    {
        if(!cin)
        {
            cin.clear();
            cin.ignore(1000, '\n');
        }
        cout << "Enter a valid number from the menu." << endl;
        cout << "Would you like to see assignments by:" << endl;
        cout << "1. Project" << endl;
        cout << "2. Employee" << endl;
        cin >> choice;
    }
    viewAssignmentsByProject(db);


    sqlite3_close(db);
    return 0;
}


int callback(void* data, int numCols, char** values, char** columnNames)
{
    for(int i = 0; i < numCols; i++)
    {
        cout << columnNames[i] << ": ";
        if(values[i] != NULL)
            cout << values[i];

        cout << endl;
    }
    cout << endl;
    return SQLITE_OK;
}

void viewAssignmentsByProject(sqlite3 * db)
{
    string query;
    sqlite3_stmt *result;
    string strLastError;
    query = "select proj_num, proj_name from project";
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if(rc != SQLITE_OK)
    {
        strLastError = sqlite3_errmsg(db);
        sqlite3_finalize(result);
        cout << "There was an error with the project query: " << strLastError << endl;
        cout << query << endl;
        return;
    }
    int columnCount = sqlite3_column_count(result);
    int i, choice;
    cout << left;
    cout << "Please choose the project you want to see the assignments for:" << endl;
    i = 1;
    do
    {
        rc = sqlite3_step(result);
        if(rc == SQLITE_DONE)
            break;
        cout << i << ". " << sqlite3_column_text(result, 0);
        cout << " - " << sqlite3_column_text(result, 1);
        cout << endl;
        i++;
        
    } while (rc == SQLITE_ROW);
    cin >> choice;
    while(!cin || choice < 1 || choice > i)
    {
        if(!cin)
        {
            cin.clear();
            cin.ignore(INT_MAX, '\n');

        }
        cout << "That is not a valid choice! Try again!" << endl;
        cin >> choice;

    }
    sqlite3_reset(result);
    for(int j = 0; j < choice; j++)
    {
        sqlite3_step(result);
    }
    string proj_num = reinterpret_cast<const char*>(sqlite3_column_text(result, 0));
    sqlite3_finalize(result);

    query = "select sum(assign_hours) as 'Total Hours', sum(assign_charge) as 'Total Charges' ";
    query += "from assignment ";
    query += "where proj_num = " + proj_num;
    query += " group by proj_num";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if(rc != SQLITE_OK)
    {
        strLastError = sqlite3_errmsg(db);
        sqlite3_finalize(result);
        cout << "There was an error with the assignment query: " << strLastError << endl;
        cout << query << endl;
        return;
    }
    columnCount = sqlite3_column_count(result);

    rc = sqlite3_step(result);
    while(rc == SQLITE_ROW)
    {
        for(int i = 0; i < columnCount; i++)
        {
            cout << sqlite3_column_name(result, i) << " - ";
            if(sqlite3_column_type(result, i) != SQLITE_NULL)
            {
                cout << sqlite3_column_text(result,i);
            }
            cout << endl;
        }
        cout << endl;
        rc = sqlite3_step(result);
    }
    sqlite3_finalize(result);
    

    
}