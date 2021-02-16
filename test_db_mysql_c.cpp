#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

int main()
{
    MYSQL *con = mysql_init(NULL);

    if (con == NULL)
    {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, "127.0.0.1", "shop1_kengqie_c", "aFhNbrjMCH",
                           "shop1_kengqie_c", 0, NULL, 0) == NULL)
    {
    }

    if (mysql_query(con, "SELECT * FROM dbshop_goods_class"))
    {
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL)
    {
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)))
    {
        for (int i = 0; i < num_fields; i++)
        {
            printf("%s ", row[i] ? row[i] : "NULL");
        }

        printf("\n");
    }

    mysql_free_result(result);
    mysql_close(con);

    exit(0);
}