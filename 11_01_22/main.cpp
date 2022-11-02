#include <iostream>
#include <string>
#include "sqlite3.h"

using namespace std;

int callback(void*, int, char**, char**);

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