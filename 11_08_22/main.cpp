

#include <iostream>
#include <string>
#include <iomanip>
#include <climits>
#include <ctime>
#include "sqlite3.h"

using namespace std;

void printMainMenu();
void viewInvoice(sqlite3 *);
void viewCustomer(sqlite3 *);
void addInvoice(sqlite3 *);
int rollback(sqlite3 *, string);
int commit(sqlite3 *);
int beginTransaction(sqlite3 *);
int mainMenu();

int main()
{
    int choice;

    sqlite3 *mydb;

    int rc;

    rc = sqlite3_open_v2("SaleCo.db", &mydb, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
        // if the database cannot be opened
        cout << "Error in connection: " << sqlite3_errmsg(mydb);
        return 1;
    }

    cout << "Welcome to SaleCo" << endl;
    choice = mainMenu();
    while (true)
    {
        switch (choice)
        {
        case 1:
            viewInvoice(mydb);
            break;
        case 2:
            viewCustomer(mydb);
            break;
        case 3:
            addInvoice(mydb);
            break;
        case -1:
        {
            // don't forget to close.
            sqlite3_close(mydb);
            return 0;
        }
        default:
            cout << "That is not a valid choice." << endl;
        }
        cout << "\n\n";
        choice = mainMenu();
    }
}

void printMainMenu()
{
    cout << "Please choose an option (enter -1 to quit):  " << endl;
    cout << "1. View an invoice" << endl;
    cout << "2. View Customer Information" << endl;
    cout << "3. Add an invoice" << endl;
    cout << "Enter Choice: ";
}

int mainMenu()
{
    int choice = 0;

    printMainMenu();
    cin >> choice;
    while ((!cin || choice < 1 || choice > 3) && choice != -1)
    {
        if (!cin)
        {
            cin.clear();
            cin.ignore(INT_MAX, '\n');
        }
        cout << "That is not a valid choice." << endl
             << endl;
        printMainMenu();
        cin >> choice;
    }
    return choice;
}

void viewInvoice(sqlite3 *db)
{
    string query = "SELECT INVOICE.INV_NUMBER, INVOICE.INV_DATE, CUSTOMER.CUS_FNAME, CUSTOMER.CUS_LNAME ";
    query += "FROM INVOICE JOIN CUSTOMER ON INVOICE.CUS_CODE = CUSTOMER.CUS_CODE;";
    sqlite3_stmt *pRes;
    string m_strLastError;
    string query2;
    string inv_number;
    string inv_date;
    string cus_fname, cus_lname;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
    {
        m_strLastError = sqlite3_errmsg(db);
        sqlite3_finalize(pRes);
        cout << "There was an error: " << m_strLastError << endl;
        return;
    }
    else
    {
        cout << "Please choose the invoice you want to see:" << endl;
        int columnCount = sqlite3_column_count(pRes);
        int i = 1, choice;
        sqlite3_stmt *pRes2;
        cout << left;
        while (sqlite3_step(pRes) == SQLITE_ROW)
        {
            cout << i << ". " << sqlite3_column_text(pRes, 0);
            cout << endl;
            i++;
        }

        // Fixed a bug where going outside the bounds of the choices would NOT
        // stop the program from proceeding and would crash with a segmentation fault.
        while (!(cin >> choice) || choice < 1 || choice > (i - 1))
        {
            if (!cin)
            {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
            }
            cout << "That is not a valid choice! Try again!" << endl;
        }

        sqlite3_reset(pRes);
        for (int i = 0; i < choice; i++)
            sqlite3_step(pRes);
        inv_number = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));
        inv_date = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 1));
        cus_fname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 2));
        cus_lname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 3));
        sqlite3_finalize(pRes);
        query2 = "SELECT PRODUCT.P_DESCRIPT as Product ,LINE.LINE_PRICE as Price, LINE.LINE_UNITS as Units ";
        query2 += "FROM LINE ";
        query2 += "JOIN PRODUCT on line.P_CODE = PRODUCT.P_CODE  ";
        query2 += "WHERE LINE.INV_NUMBER = '" + inv_number + "';";

        if (sqlite3_prepare_v2(db, query2.c_str(), -1, &pRes2, NULL) != SQLITE_OK)
        {
            m_strLastError = sqlite3_errmsg(db);
            sqlite3_finalize(pRes2);
            cout << "There was an error: " << m_strLastError << endl;
            return;
        }
        else
        {
            cout << "Invoice #: " << inv_number << endl;
            cout << "Invoice Date: " << inv_date << endl;
            cout << "Customer: " << cus_fname << " " << cus_lname << endl;
            columnCount = sqlite3_column_count(pRes2);
            cout << left;
            for (int i = 0; i < columnCount; i++)
            {
                cout << "|" << setw(25) << sqlite3_column_name(pRes2, i);
            }
            cout << "|" << endl;

            while (sqlite3_step(pRes2) == SQLITE_ROW)
            {
                for (int i = 0; i < columnCount; i++)
                {
                    if (sqlite3_column_type(pRes2, i) != SQLITE_NULL)
                        cout << "|" << setw(25) << sqlite3_column_text(pRes2, i);
                    else
                        cout << "|" << setw(25) << " ";
                }
                cout << "|" << endl;
            }
            sqlite3_finalize(pRes2);
        }
    }
}

void viewCustomer(sqlite3 *db)
{

    string cusID, firstName, lastName, initial, phoneNum, areaCode, balance;
    balance = "$";

    // Note I decided to use query string concatenation to simplify the programatic process
    // And also grab the customer code by itself, for use in the second query.
    string query = "SELECT CUS_CODE || \" - \" || CUS_LNAME || \", \" || CUS_FNAME AS 'CUSTOMER', CUS_CODE FROM CUSTOMER ORDER BY CUS_CODE ASC";
    sqlite3_stmt *firstStatement;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &firstStatement, NULL) != SQLITE_OK)
    {
        // When an error occurs preparing the first statement...
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(firstStatement);
        cout << "There was an error: " << err << endl;
        return;
    }
    else
    {
        cout << "Please choose the customer you want to see:\n";
        int choice;
        int i = 1;

        while (sqlite3_step(firstStatement) == SQLITE_ROW)
        {
            // print first query results.
            cout << i++ << ". " << sqlite3_column_text(firstStatement, 0) << endl;
        }

        // get input and test for improper values.
        while (!(cin >> choice) || (choice > (i - 1)) || choice < 1)
        {
            if (!cin)
            {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
            }

            cout << "That is not a valid choice! Try again!" << endl;
        }

        // reset the statement and iterate to the choice.
        sqlite3_reset(firstStatement);
        for (int i = 0; i < choice; i++)
        {
            sqlite3_step(firstStatement);
        }

        // retrieve our customer id, then craft our new query and prepare our next statement.
        cusID = reinterpret_cast<const char *>(sqlite3_column_text(firstStatement, 1));
        sqlite3_finalize(firstStatement);

        string query2 = "SELECT * FROM CUSTOMER WHERE CUS_CODE = ";
        query2 += cusID += ";";

        sqlite3_stmt *secondStatement;
        if (sqlite3_prepare_v2(db, query2.c_str(), -1, &secondStatement, NULL) != SQLITE_OK)
        {
            // When an error occurs preparing the second statement...
            string err = sqlite3_errmsg(db);
            sqlite3_finalize(secondStatement);
            cout << "There was an error: " << err << endl;
            return;
        }

        else
        {
            // Since we have verified programatically that we should have a result here,
            // we step once without checking that it is valid.
            sqlite3_step(secondStatement);

            lastName = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 1));
            firstName = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 2));

            // we have to test to see if this column that CAN be empty is.
            // otherwise it will crash if you try to get results from someone without
            // an initial!
            if (!(sqlite3_column_type(secondStatement, 3) == SQLITE_NULL))
            {
                initial = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 3));
            }

            areaCode = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 4));
            phoneNum = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 5));
            balance += reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 6));

            sqlite3_finalize(secondStatement);

            cout << "----Customer Information----\n";
            if (initial != "")
                cout << "Name: " << firstName << " " << initial << ". " << lastName;
            else
                cout << "Name: " << firstName << " " << lastName;
            cout << "\nPhone Number: (" << areaCode << ")" << phoneNum
                 << "\nBalance: " << balance;

            return;
        }
    }
}

void addInvoice(sqlite3 *db)
{
    int rc = beginTransaction(db);
    if (rc != SQLITE_OK)
    {
        return;
    }
    string query = "select cus_code, cus_fname || ' ' || cus_lname as name ";
    query += "from customer order by cus_code";
    sqlite3_stmt *result;
    string error;
    string cus_code;
    int inv_number;

    char formatDate[80];
    time_t currentDate = time(NULL);
    strftime(formatDate, 80, "%F", localtime(&currentDate)); // for date and time "%F %T"
    string inv_date(formatDate);
    double total = 0;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        rollback(db, query);
        sqlite3_finalize(result);
        return;
    }
    else
    {
        cout << "Please choose the customer for the invoice:" << endl;
        int i = 0, choice;
        do
        {
            if (sqlite3_column_type(result, 0) != SQLITE_NULL)
            {
                cout << ++i << ". " << sqlite3_column_text(result, 0)
                     << " - " << sqlite3_column_text(result, 1);
                cout << endl;
            }
            rc = sqlite3_step(result);
        } while (rc == SQLITE_ROW);
        cin >> choice;

        while (!cin || choice < 1 || choice > i)
        {
            if (!cin)
            {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
            }
            cout << "That is not a valid choice! Try again!" << endl;
            cin >> choice;
        }
        sqlite3_reset(result);
        for (int j = 0; j < choice; j++)
            sqlite3_step(result);
        cus_code = reinterpret_cast<const char *>(sqlite3_column_text(result, 0));
        sqlite3_finalize(result);
    }
    query = "insert into invoice (cus_code, inv_date) values (";
    query += cus_code;
    query += ", '";
    query += inv_date;
    query += "');";

    rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        rollback(db, query);
        return;
    }
    inv_number = sqlite3_last_insert_rowid(db);
    char cont = 'y';
    query = "SELECT p_code, p_descript, p_qoh, p_price from product";
    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(result);
        rollback(db, query);
        return;
    }
    int lineNum = 1;
    while (cont == 'y' || cont == 'Y')
    {
        sqlite3_reset(result);
        cout << "Please choose a product for the invoice: " << endl;
        int i = 0, choice;
        do
        {
            if (sqlite3_column_type(result, 0) != SQLITE_NULL)
            {
                cout << ++i << ". " << sqlite3_column_text(result, 0)
                     << " - " << sqlite3_column_text(result, 1);
                cout << endl;
            }
            rc = sqlite3_step(result);
        } while (rc == SQLITE_ROW);
        cin >> choice;
        while (!cin || choice < 1 || choice > i)
        {
            if (!cin)
            {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
            }
            cout << "That is not a valid choice! Try again!" << endl;
            cin >> choice;
        }
        for (int j = 0; j < choice; j++)
            sqlite3_step(result);
        string prod_code = reinterpret_cast<const char *>(sqlite3_column_text(result, 0));
        int qoh = sqlite3_column_int(result, 2);
        double price = sqlite3_column_double(result, 3);
        int quantity = 0;
        cout << "How many would you like? (Quantity on Hand: " << qoh << "): ";
        cin >> quantity;
        cout << endl;
        while (!cin || quantity <= 0 || quantity > qoh)
        {
            if (!cin)
            {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
            }
            if (quantity == -1)
            {
                rollback(db, "");
                sqlite3_finalize(result);
                return;
            }
            cout << "That is not a valid quantity.  Please try again or enter -1 to quit and rollback." << endl;
            cout << "How many would you like? (Quantity on Hand: " << qoh << "): ";
            cin >> quantity;
        }
        total += quantity * price;
        query = "insert into line values (";
        query += to_string(inv_number) + ", ";
        query += to_string(lineNum++) + ", ";
        query += "'" + prod_code + "', ";
        query += to_string(quantity) + ", ";
        query += to_string(price) + ");";
        rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            rollback(db, query);
            sqlite3_finalize(result);
            return;
        }
        query = "update product set p_qoh = p_qoh - " + to_string(quantity) + " where p_code = '" + prod_code + "';";
        rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            rollback(db, query);
            sqlite3_finalize(result);
            return;
        }

        cout << "Would you like to enter another product? Y or N";
        cin >> cont;
    }
    sqlite3_finalize(result);
    query = "update customer set cus_balance = cus_balance + " + to_string(total) + " where cus_code = " + cus_code + ";";
    rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        rollback(db, query);
        return;
    }

    rc = commit(db);
    if(rc == SQLITE_OK)
    {
        cout << "Successfully inserted invoice " << inv_number << endl;
    }
    // cout << inv_date << endl;
}

int rollback(sqlite3 *db, string query)
{

    string error = sqlite3_errmsg(db);
    // sqlite3_finalize(result);
    cout << "There was an error: " << error << endl;
    cout << query << endl;
    char *err;
    string query1 = "rollback";
    int rc = sqlite3_exec(db, query1.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK)
    {
        cout << "There was an error on rollback: " << err << endl;
        sqlite3_free(err);
    }
    return rc;
}

int commit(sqlite3 *db)
{
    char *err;
    string query = "commit";
    int rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK)
    {
        cout << "There was an error on commit: " << err << endl;
        sqlite3_free(err);
    }
    return rc;
}

int beginTransaction(sqlite3 *db)
{
    char *err;
    string query = "begin transaction";
    int rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK)
    {
        cout << "There was an error on begin transaction: " << err << endl;
        sqlite3_free(err);
    }
    return rc;
}