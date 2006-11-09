/*
 * This file contains misc functions for dealing with armor
 */

#include <curses.h>
#include "rogue.h"

/*
 * wear:
 *	The player wants to wear something, so let him/her put it on.
 */

wear()
{
    register struct linked_list *item;
    register struct object *obj;

    if (cur_armor != NULL)
    {
	addmsg("You are already wearing some");
	if (!terse)
	    addmsg(".  You'll have to take it off first");
	addmsg("!");
	endmsg();
	after = FALSE;
	return;
    }

    /* What does player want to wear? */
    if ((item = get_item("wear", ARMOR)) == NULL)
	return;

    obj = (struct object *) ldata(item);
    if (obj->o_type != ARMOR) {
	 msg("You can't wear that!");
	 return;
    }
    waste_time();
    addmsg(terse ? "W" : "You are now w");
    msg("earing %s.", armors[obj->o_which].a_name);
    cur_armor = obj;
    obj->o_flags |= ISKNOW;
}

/*
 * take_off:
 *	Get the armor off of the players back
 */

take_off()
{
    register struct object *obj;

    if ((obj = cur_armor) == NULL)
    {
	msg("%s wearing armor!", terse ? "Not" : "You aren't");
	return;
    }
    if (!dropcheck(cur_armor))
	return;
    cur_armor = NULL;
    addmsg(terse ? "Was" : "You used to be");
    msg(" wearing %c) %s.", pack_char(obj), inv_name(obj, TRUE));
}

/*
 * waste_time:
 *	Do nothing but let other things happen
 */

waste_time()
{
    if (inwhgt)			/* if from wghtchk then done */
	return;
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    do_daemons(AFTER);
    do_fuses(AFTER);
}
