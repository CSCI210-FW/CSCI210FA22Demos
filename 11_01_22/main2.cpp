#include <iostream>
#include <string>
#include "sqlite3.h"

using namespace std;

int movieCallback(void*, int, char**, char**);
int actorCallback(void*, int, char**, char**);

int main()
{
    sqlite3 * db;
    int rc;

    rc  = sqlite3_open_v2("IMDB.db", &db, SQLITE_OPEN_READWRITE, NULL);
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

    string query = "select distinct movie.name as 'Movie', movie.year as 'Year', movie.id ";
    query += "from movie ";
    query += "where movie.name like \"Shrek%\"";
    cout << query << endl;
    char * err;
    rc = sqlite3_exec(db, query.c_str(), movieCallback, db, &err);
    if(rc != SQLITE_OK)
    {
        cout << "There was an error - select callback: " << err << endl;
        sqlite3_free(err);
    }

    sqlite3_close(db);
    return 0;
}


int movieCallback(void* data, int numCols, char** values, char** columnNames)
{
    for(int i = 0; i < numCols - 1; i++)
    {
        cout << columnNames[i] << ": ";
        if(values[i] != NULL)
            cout << values[i]; 

        cout << endl;
    }
    cout << "Cast:" << endl;
    sqlite3 * db = (sqlite3 *) data;
    string query = "select actor.first_name || ' ' || actor.last_name || ' - ' || c.role as actorinfo ";
    query += "from actor join cast as c on actor.id = c.actor_id ";
    query += "where c.movie_id = ";
    query += values[numCols -1];
    char * err;
    int rc = sqlite3_exec(db, query.c_str(), actorCallback, NULL, &err);
    if(rc != SQLITE_OK)
    {
        cout << "There was an error - actor callback: " << err << endl;
        cout << query << endl;
        sqlite3_free(err);
        return SQLITE_ERROR;
    }
    cout << endl;
    return SQLITE_OK;
}

int actorCallback(void* data, int numCols, char** values, char** columnNames)
{
    if(values[0] != NULL)
        cout << values[0];
    cout << endl;
    return SQLITE_OK;
}