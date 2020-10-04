#include "curses.h"
#include "rogue.h"
  
struct cell {
  int y_pos;
  int x_pos;
  };
struct bordercells {
   int num_pos;		/* number of frontier cells next to you */
   struct cell conn[4];	/* the y,x position of above cell */
 } borderC;
  
char *frontier, *bits;
char *moffset(), *foffset();
int lines, cols;
  
  
/*
* domaze:
*Draw the maze on this level.
*/
do_maze()
{
  reg int i, least;
  reg struct room *rp;
  reg struct linked_list *item;
  reg struct object *obj;
  reg struct thing *mp;
  bool treas;
  coord tp;
  
  for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
  	rp->r_nexits = 0;		/* no exits */
  	rp->r_flags = ISGONE;		/* kill all rooms */
  	rp->r_fires = 0;		/* no fires */
  }
  rp = &rooms[0];				/* point to only room */
  rp->r_flags = ISDARK;			/* mazes always dark */
  rp->r_pos.x = 0;			/* room fills whole screen */
  rp->r_pos.y = 1;
  rp->r_max.x = COLS - 1;
  rp->r_max.y = LINES - 3;
  draw_maze();				/* put maze into window */
  /*
   * add some gold to make it worth looking for 
   */
  item = spec_item(GOLD, NULL, NULL, NULL);
  obj = OBJPTR(item);
  obj->o_count *= (rnd(10) + 1);		/* add in one large hunk */
  attach(lvl_obj, item);
  rnd_pos(rp, &tp);
  mvaddch(tp.y, tp.x, GOLD);
  obj->o_pos = tp;
  /*
   * add in some food to make sure he has enough
   */
  item = spec_item(FOOD, NULL, NULL, NULL);
  obj = OBJPTR(item);
  attach(lvl_obj, item);
  rnd_pos(rp, &tp);
  mvaddch(tp.y, tp.x, FOOD);
  obj->o_pos = tp;
  if (rnd(100) < 5) {			/* 5% for treasure maze level */
  	treas = TRUE;
  	least = 6;
  	debug("treasure maze");
  }
  else {					/* normal maze level */
  	least = 1;
  	treas = FALSE;
  }
  for (i = 0; i < level + least; i++) {
  	if (!treas && rnd(100) < 50)	/* put in some little buggers */
  		continue;
  	/*
  	 * Put the monster in
  	 */
  	item = new_item(sizeof *mp);
  	mp = THINGPTR(item);
  	do {
  		rnd_pos(rp, &tp);
  	} until(mvwinch(stdscr, tp.y, tp.x) == FLOOR);
 
	new_monster(item, randmonster(FALSE, FALSE), &tp, FALSE);
  	/*
  	 * See if we want to give it a treasure to carry around.
  	 */
  	if (rnd(100) < monsters[mp->t_index].m_carry)
  		attach(mp->t_pack, new_thing());
 
 	/*
	 * If it carries gold, give it some
  	 */
  	if (on(*mp, CARRYGOLD)) {
  		reg struct object *cur;
 
		item = spec_item(GOLD, NULL, NULL, NULL);
  		obj = OBJPTR(item);
  		obj->o_count = GOLDCALC + GOLDCALC + GOLDCALC;
  		obj->o_pos = mp->t_pos;
  		attach(mp->t_pack, item);
  	}
 
    }
  }
  
 /*
  * draw_maze:
*Generate and draw the maze on the screen
*/
draw_maze()
 {
   reg int i, j, more;
   reg char *ptr;
 
     lines = (LINES - 3) / 2;
     cols = (COLS - 1) / 2;
     bits = ALLOC((LINES - 3) * (COLS - 1));
     frontier = ALLOC(lines * cols);
     ptr = frontier;
     while (ptr < (frontier + (lines * cols)))
  	*ptr++ = TRUE;
        for (i = 0; i < LINES - 3; i++) {
          for (j = 0; j < COLS - 1; j++) {
  		if (i % 2 == 1 && j % 2 == 1)
  	        	*moffset(i, j) = FALSE;		/* floor */
  		else
  			*moffset(i, j) = TRUE;		/* wall */
  	}
    }
  for (i = 0; i < lines; i++) {
  	for (j = 0; j < cols; j++) {
  		do
  			more = findcells(i,j);
  		while(more != 0);
  	}
  }
  crankout();
  FREE(frontier);
  FREE(bits);
 }
  
/*
 * moffset:
 *Calculate memory address for bits
 */
 char *
 moffset(y, x)
 int y, x;
 {
  
return (bits + (y * (COLS - 1)) + x);
 }
  
 /*
  * foffset:
  *	Calculate memory address for frontier
  */
 char *
 foffset(y, x)
 int y, x;
 {
  
return (frontier + (y * cols) + x);
}
  
 /*
  * findcells:
  *	Figure out cells to open up 
  */
 findcells(y,x)
 reg int x, y;
 {
  reg int rtpos, i;
 
  *foffset(y, x) = FALSE;
      borderC.num_pos = 0;
   if (y < lines - 1) {				/* look below */
  	if (*foffset(y + 1, x)) {
  		borderC.conn[borderC.num_pos].y_pos = y + 1;
  		borderC.conn[borderC.num_pos].x_pos = x;
  		borderC.num_pos += 1;
  	}
  }
  if (y > 0) {					/* look above */
  	if (*foffset(y - 1, x)) {
  		borderC.conn[borderC.num_pos].y_pos = y - 1;
  		borderC.conn[borderC.num_pos].x_pos = x;
  		borderC.num_pos += 1;
  
    }
  }
  if (x < cols - 1) {				/* look right */ if (*foffset(y, x + 1)) {
  		borderC.conn[borderC.num_pos].y_pos = y;
  		borderC.conn[borderC.num_pos].x_pos = x + 1;
  		borderC.num_pos += 1;
  }
 }
 if (x > 0) {					/* look left */
 	if (*foffset(y, x - 1)) {
  		borderC.conn[borderC.num_pos].y_pos = y;
  		borderC.conn[borderC.num_pos].x_pos = x - 1;
  		borderC.num_pos += 1;
 
	}
  }
  if (borderC.num_pos == 0)		/* no neighbors available */
  	return 0;
  else {
  	i = rnd(borderC.num_pos);
  	rtpos = borderC.num_pos - 1;
  	rmwall(borderC.conn[i].y_pos, borderC.conn[i].x_pos, y, x);
  	return rtpos;
  }
 }
  
 /*
  * rmwall:
  *	Removes appropriate walls from the maze
  */
rmwall(newy, newx, oldy, oldx)
int newy, newx, oldy, oldx;
 {
   reg int xdif,ydif;
  	
   xdif = newx - oldx;
   ydif = newy - oldy;
  
  *moffset((oldy * 2) + ydif + 1, (oldx * 2) + xdif + 1) = FALSE;
  findcells(newy, newx);
 }
  
  
/*
 * crankout:
 *	Does actual drawing of maze to window
 */
crankout()
  {
    reg int x, y, i;
  
    for (y = 0; y < LINES - 3; y++) {
        move(y + 1, 0);
  	for (x = 0; x < COLS - 1; x++) {
  		if (*moffset(y, x)) {		/* here is a wall */
  			if(y==0 || y==LINES-4) /* top or bottom line */
  				addch('-');
  			else if(x==0 || x==COLS-2) /* left | right side */
  				addch('|');
  			else if (y % 2 == 0 && x % 2 == 0) {
  			if(*moffset(y, x-1) || *moffset(y, x+1))
  				addch('-');
  			else
  				addch('|');
  			}
         		else if (y % 2 == 0)
  			addch('-');
  	        	else
  			addch('|');
  		}
                 	else
  		        addch(FLOOR);
  	}
    }
  }
