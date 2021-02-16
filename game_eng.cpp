
// Using gafq as a C library
extern "C"
{
#include <gafq.h>
#include <gafqlib.h>
#include <gauxlib.h>
}

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    int status, result, i;
    double sum;
    gafq_State *L;

    /*
     * All Lua contexts are held in this structure. We work with it almost
     * all the time.
     */
    L = gafqL_newstate();

    gafqL_openlibs(L); /* Load Lua libraries */

    /* Load the file containing the script we are going to run */
    status = gafqL_loadfile(L, "script.gafq");
    if (status)
    {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        fprintf(stderr, "Couldn't load file: %s\n", gafq_tostring(L, -1));
        exit(1);
    }

    /*
     * Ok, now here we go: We pass data to the gafq script on the stack.
     * That is, we first have to prepare Lua's virtual stack the way we
     * want the script to receive it, then ask Lua to run it.
     */
    gafq_newtable(L); /* We will pass a table */

    /*
     * To put values into the table, we first push the index, then the
     * value, and then call gafq_rawset() with the index of the table in the
     * stack. Let's see why it's -3: In Lua, the value -1 always refers to
     * the top of the stack. When you create the table with gafq_newtable(),
     * the table gets pushed into the top of the stack. When you push the
     * index and then the cell value, the stack looks like:
     *
     * <- [stack bottom] -- table, index, value [top]
     *
     * So the -1 will refer to the cell value, thus -3 is used to refer to
     * the table itself. Note that gafq_rawset() pops the two last elements
     * of the stack, so that after it has been called, the table is at the
     * top of the stack.
     */
    for (i = 1; i <= 5; i++)
    {
        gafq_pushnumber(L, i);     /* Push the table index */
        gafq_pushnumber(L, i * 2); /* Push the cell value */
        gafq_rawset(L, -3);        /* Stores the pair in the table */
    }

    /* By what name is the script going to reference our table? */
    gafq_setglobal(L, "foo");

    /* Ask Lua to run our little script */
    result = gafq_pcall(L, 0, GAFQ_MULTRET, 0);
    if (result)
    {
        fprintf(stderr, "Failed to run script: %s\n", gafq_tostring(L, -1));
        exit(1);
    }

    /* Get the returned value at the top of the stack (index -1) */
    sum = gafq_tonumber(L, -1);

    printf("Script returned: %.0f\n", sum);

    gafq_pop(L, 1); /* Take the returned value out of the stack */
    gafq_close(L);  /* Cya, Lua */

    return 0;
}