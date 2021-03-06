/*
 * All the daemon and fuse functions are in here
 *
 */

#include "curses.h"
#include "rogue.h"

/*
 * doctor:
 *	A healing daemon that restors hit points after rest
 */

doctor(tp)
struct thing *tp;
{
    register int ohp;
    register short limit, new_points;
    register struct stats *curp; /* current stats pointer */
    register struct stats *maxp; /* max stats pointer */

    curp = &(tp->t_stats);
    maxp = &(tp->maxstats);
    if (curp->s_hpt == maxp->s_hpt || on(*tp, ISINWALL)) {
	tp->t_quiet = 0;
	return;
    }
    tp->t_quiet++;
    switch (tp->t_ctype) {
	when C_MAGICIAN:
	    limit = 4 - curp->s_lvl/2;
	    new_points = curp->s_lvl;
	when C_THIEF:
	    limit = 8 - curp->s_lvl;
	    new_points = curp->s_lvl - 2;
	when C_CLERIC:
	    limit = 8 - curp->s_lvl;
	    new_points = curp->s_lvl - 3;
	when C_FIGHTER:
	    limit = 16 - curp->s_lvl*2;
	    new_points = curp->s_lvl - 5;
	when C_MONSTER:
	    limit = 16 - curp->s_lvl;
	    new_points = curp->s_lvl - 6;
	otherwise:
	    debug("What a strange character you are!");
	    return;
    }
    ohp = curp->s_hpt;
    if (off(*tp, HASDISEASE)) {
	if (curp->s_lvl < 8) {
	    if (tp->t_quiet > limit)
		curp->s_hpt++;
	}
	else
	    if (tp->t_quiet >= 3)
		curp->s_hpt += rnd(new_points)+1;
    }
    if (tp == &player) {
	if (ISRING(LEFT_1, R_REGEN)) curp->s_hpt++;
	if (ISRING(LEFT_2, R_REGEN)) curp->s_hpt++;
	if (ISRING(LEFT_3, R_REGEN)) curp->s_hpt++;
	if (ISRING(LEFT_4, R_REGEN)) curp->s_hpt++;
	if (ISRING(RIGHT_1, R_REGEN)) curp->s_hpt++;
	if (ISRING(RIGHT_2, R_REGEN)) curp->s_hpt++;
	if (ISRING(RIGHT_3, R_REGEN)) curp->s_hpt++;
	if (ISRING(RIGHT_4, R_REGEN)) curp->s_hpt++;
    }
    if (on(*tp, ISREGEN))
	curp->s_hpt += curp->s_lvl/5 + 1;
    if (ohp != curp->s_hpt) {
	if (curp->s_hpt >= maxp->s_hpt) {
	    curp->s_hpt = maxp->s_hpt;
	    if (off(*tp, WASTURNED) && on(*tp, ISFLEE) && tp != &player) {
		turn_off(*tp, ISFLEE);
		tp->t_oldpos = tp->t_pos;	/* Start our trek over */
	    }
	}
	tp->t_quiet = 0;
    }
}

/*
 * Swander:
 *	Called when it is time to start rolling for wandering monsters
 */

swander()
{
    daemon(rollwand, 0, BEFORE);
}

/*
 * rollwand:
 *	Called to roll to see if a wandering monster starts up
 */

rollwand()
{
    static int between = 0;

    if (++between >= 4)
    {
	/* Theives may not awaken a monster */
	if ((roll(1, 6) == 4) &&
	   ((player.t_ctype != C_THIEF) || (rnd(30) >= pstats.s_dext))) {
	    if (levtype != POSTLEV)
	        wanderer();
	    kill_daemon(rollwand);
	    fuse(swander, 0, WANDERTIME, BEFORE);
	}
	between = 0;
    }
}

/*
 * unconfuse:
 *	Release the poor player from his confusion
 */

unconfuse()
{
    turn_off(player, ISHUH);
    msg("You feel less confused now.");
}


/*
 * unscent:
 *	turn of extra smelling ability
 */

unscent()
{
    turn_off(player, CANSCENT);
    msg("The smell of monsters goes away.");
}


/*
 * scent:
 *	give back the players sense of smell
 */

scent()
{
    turn_off(player, ISUNSMELL);
    msg("You begin to smell the damp dungeon air again.");
}


/*
 * unhear:
 *	player doesn't have extra hearing any more
 */

unhear()
{
    turn_off(player, CANHEAR);
    msg("The sounds of monsters fades away.");
}


/*
 * hear:
 *	return the players sense of hearing
 */

hear()
{
    turn_off(player, ISDEAF);
    msg("You can hear again.");
}


/*
 * unsee:
 *	He lost his see invisible power
 */

unsee()
{
    if (!ISWEARING(R_SEEINVIS)) {
	turn_off(player, CANSEE);
	msg("The tingling feeling leaves your eyes.");
    }
}

/*
 * unstink:
 *	Remove to-hit handicap from player
 */

unstink()
{
    turn_off(player, HASSTINK);
}

/*
 * unclrhead:
 *	Player is no longer immune to confusion
 */

unclrhead()
{
    turn_off(player, ISCLEAR);
    msg("The blue aura about your head fades away.");
}

/*
 * unphase:
 *	Player can no longer walk through walls
 */

unphase()
{
    turn_off(player, CANINWALL);
    msg("Your dizzy feeling leaves you.");
    if (!step_ok(hero.y, hero.x, NOMONST, &player)) 
	death(D_PETRIFY);
}

/*
 * sight:
 *	He gets his sight back
 */

sight()
{
    if (on(player, ISBLIND))
    {
	extinguish(sight);
	turn_off(player, ISBLIND);
	light(&hero);
	msg("The veil of darkness lifts.");
    }
}

/*
 * res_strength:
 *	Restore player's strength
 */

res_strength()
{

    /* If lost_str is non-zero, restore that amount of strength,
     * else all of it 
     */
    if (lost_str) {
	chg_str(lost_str, FALSE, FALSE);
	lost_str = 0;
    }

    /* Otherwise, put player at the maximum strength */
    else {
	pstats.s_str = max_stats.s_str + ring_value(R_ADDSTR) + 
		(on(player, POWERSTR) ? 10 : 0);
    }

    updpack(TRUE);
}

/*
 * nohaste:
 *	End the hasting
 */

nohaste()
{
    turn_off(player, ISHASTE);
    msg("You feel yourself slowing down.");
}

/*
 * noslow:
 *	End the slowing
 */

noslow()
{
    turn_off(player, ISSLOW);
    msg("You feel yourself speeding up.");
}

/*
 * suffocate:
 *	If this gets called, the player has suffocated
 */

suffocate()
{
    death(D_SUFFOCATION);
}

/*
 * digest the hero's food
 */
stomach()
{
    register int oldfood, old_hunger;
    register int amount;
    int power_scale;

    old_hunger = hungry_state;
    if (food_left <= 0)
    {
	/*
	 * the hero is fainting
	 */
	if (no_command || rnd(100) > 20)
	    return;
	no_command = rnd(8)+4;
	if (!terse)
	    addmsg("You feel too weak from lack of food.  ");
	msg("You faint.");
	running = FALSE;
	count = 0;
	hungry_state = F_FAINT;
    }
    else
    {
	oldfood = food_left;
	amount =	ring_eat(LEFT_1) + ring_eat(LEFT_2) +
			ring_eat(LEFT_3) + ring_eat(LEFT_4) +
	    		ring_eat(RIGHT_1) + ring_eat(RIGHT_2) +
			ring_eat(RIGHT_3) + ring_eat(RIGHT_4) + 
			foodlev;

	if (on(player, SUPEREAT))
	    amount *= 2;

	if (on(player, POWEREAT)) {
	    amount += 40;
	    turn_off(player, POWEREAT);
	}

	power_scale =	on(player, POWERDEXT) + on(player, POWERSTR) +
		on(player, POWERWISDOM) + on(player, POWERINTEL) +
		on(player, POWERCONST) + 1;

	food_left -= amount * power_scale;

	if (food_left < MORETIME && oldfood >= MORETIME)
	{
	    msg("You are starting to feel weak.");
	    hungry_state = F_WEAK;
	}
	else if (food_left < 2 * MORETIME && oldfood >= 2 * MORETIME)
	{
	    msg(terse ? "Getting hungry." : "You are starting to get hungry.");
	    hungry_state = F_HUNGRY;
	}

    }
    if (old_hunger != hungry_state) 
	updpack(TRUE);
    wghtchk();
}
/*
 * daemon for curing the diseased
 */
cure_disease()
{
    turn_off(player, HASDISEASE);
    if (off (player, HASINFEST))
	msg(terse ? "You feel yourself improving."
		: "You begin to feel yourself improving again.");
}

/*
 * daemon for adding back dexterity
 */
un_itch()
{
    if (lost_dext) {
	chg_dext(lost_dext, FALSE, FALSE);
	lost_dext = 0;
	turn_off(player, HASITCH);
    }
}
/*
 * appear:
 *	Become visible again
 */
appear()
{
    turn_off(player, ISINVIS);
    PLAYER = VPLAYER;
    msg("The tingling feeling leaves your body.");
    light(&hero);
}

/*
 * unelectrify:
 *	armor stops shooting off sparks
 */
unelectrify()
{
    turn_off(player, ISELECTRIC);
    msg("The sparks and violet glow from your body fade away.");
    light(&hero);
}

/*
 * unshero:
 *	super heroism wears off, now do nasty effects
 */
unshero()
{
    msg("Your feeling of invulnerability goes away.");
    turn_off(player, SUPERHERO);
    chg_str(-11, FALSE, FALSE);
    chg_dext(-6, FALSE, FALSE);
    food_left -= HEROTIME + rnd(HEROTIME);
    no_command += 5 + rnd(5);
    msg("You fall asleep.");
}

/*
 * unbhero:
 *	super heroism wears off, but no bad effects
 */
unbhero()
{
    msg("Your feeling of invincibility goes away.");
    turn_off(player, SUPERHERO);
    chg_str(-10, FALSE, FALSE);
    chg_dext(-5, FALSE, FALSE);
}

/*
 * unxray:
 *	x-ray vision wears off
 */
unxray()
{
}

/*
 * undisguise:
 *	player stops looking like a monster
 */
undisguise()
{
    msg("Your skin feels itchy for a moment.");
    turn_off(player, ISDISGUISE);
    PLAYER = VPLAYER;
    light(&hero);
}

/*
 * shero:
 *	restore lost abilities from cursed potion of shero
 */
shero()
{
    msg("You feel normal again.");
    chg_str(2);
    chg_dext(2);
    turn_off(player, ISUNHERO);
}
