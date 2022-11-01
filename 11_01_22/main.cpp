#include <iostream>
#include <string>
#include "sqlite3.h"

using namespace std;


int main()
{
    sqlite3 * db;
    int rc;

    rc  = sqlite3_open_v2("ConstructCo1.db", &db, SQLITE_OPEN_READWRITE, NULL);
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


    sqlite3_close(db);
    return 0;
}