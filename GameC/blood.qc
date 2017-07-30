/*
	GieV: I don't know who to credit for this. Several tutorials are using this code,
		as it is quite generic. I got it from http://www.insideqc.com/qctut/qctut-28.shtml, by 'Kilbert'.
*/

﻿void() blood = {
    // Don't spray blood until part of us stops
    if(self.velocity != '0 0 0') {
        /* The numbers in blue is the number for color as it is in the Quake Palette.
        You can also get rainbow blood by changing those numbers to random()*254.*/
        particle(self.origin,'0 0 0',75,30);
        particle(self.origin,'0 0 5',73,30);
        particle(self.origin,'0 0 5',69,30);

        self.think = blood;  // The must have to this tut. Makes it so particles keep comin.
        self.nextthink = time + 0.01;
    }
    // or...when we stop
    else
    {
// Check to see if it is a head
        if((self.frame == 0) && (self.view_ofs == '0 0 8'))
            self.nextthink = -1;
        else
        {
            self.think = SUB_Remove;
            self.nextthink = time + 10 + random()*10;
        }
    }
};
