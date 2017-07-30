/*
	GieV: This code is taken/cleaned up from http://www.insideqc.com/qctut/qctut-77.shtml,
		and it was originally written by 'Tux'.
*/

void() blue_particle_trail = {
    particle(self.origin, '0 0 0', 210, 140);
    self.think = blue_particle_trail;
    self.nextthink = time + 0.1;
};

void() zap_splash = {
    local entity zap;
    zap = findradius(self.origin, 100000);

    while (zap) {
        if (zap.classname == "splash") {
            remove(zap);
        }

        zap = zap.chain;
    }

    remove(self);
};

void(vector place, vector splashvel) SpawnSplash = {
    local entity splash, sp_kill;
    splash = spawn();
    splash.origin = place;
    setmodel(splash, "models/null.spr");
    setsize(splash, VEC_HULL_MIN, VEC_HULL_MAX);
    splash.solid = SOLID_TRIGGER;
    splash.classname = "splash";
    splash.movetype = MOVETYPE_TOSS;
    splash.velocity = splashvel;
    splash.think = blue_particle_trail;
    splash.nextthink = time;

    sp_kill = spawn();
    sp_kill.origin = place;
    sp_kill.think = zap_splash;
    sp_kill.nextthink = time + 0.7;
};

void(vector spot, vector splashsize) Splash = {
    local vector splash1, splash2, splash3, splash4, splash5, splash6, splash7, splash8;
    local vector vsplash1, vsplash2, vsplash3, vsplash4, vsplash5, vsplash6, vsplash7, vsplash8;
    local vector last;
    local float holder;

    splash1 = spot + '50 0 16';
    splash2 = spot + '-50 0 16';
    splash3 = spot + '0 50 16';
    splash4 = spot + '0 -50 16';
    splash5 = spot + '50 50 16';
    splash6 = spot + '-50 -50 16';
    splash7 = spot + '50 -50 16';
    splash8 = spot + '-50 50 16';

    holder = splashsize_x + splashsize_y;
    holder = holder / 2;

    vsplash1 = normalize(splash1 - spot);
    vsplash1 = vsplash1 * holder;
    vsplash1_z = splashsize_z * -1;

    vsplash2 = normalize(splash2 - spot);
    vsplash2 = vsplash2 * holder;
    vsplash2_z = splashsize_z * -1;

    vsplash3 = normalize(splash3 - spot);
    vsplash3 = vsplash3 * holder;
    vsplash3_z = splashsize_z * -1;

    vsplash4 = normalize(splash4 - spot);
    vsplash4 = vsplash4 * holder;
    vsplash4_z = splashsize_z * -1;

    vsplash5 = normalize(splash5 - spot);
    vsplash5 = vsplash5 * holder;
    vsplash5_z = splashsize_z * -1;

    vsplash6 = normalize(splash6 - spot);
    vsplash6 = vsplash6 * holder;
    vsplash6_z = splashsize_z * -1;

    vsplash7 = normalize(splash7 - spot);
    vsplash7 = vsplash7 * holder;
    vsplash7_z = splashsize_z * -1;

    vsplash8 = normalize(splash8 - spot);
    vsplash8 = vsplash1 * holder;
    vsplash8_z = splashsize_z * -1;

    last = '0 0 0';
    last_z = splashsize_z * -1;
    last = last * 0.5;

    SpawnSplash(splash1, vsplash1 * 0.5);
    SpawnSplash(splash2, vsplash2 * 0.5);
    SpawnSplash(splash3, vsplash3 * 0.5);
    SpawnSplash(splash4, vsplash4 * 0.5);
    SpawnSplash(splash5, vsplash5 * 0.5);
    SpawnSplash(splash6, vsplash6 * 0.5);
    SpawnSplash(splash7, vsplash7 * 0.5);
    SpawnSplash(splash8, vsplash8 * 0.5);
    SpawnSplash(spot, last);
};

void() CheckSplash = {
    local entity checker;
    local float check;
    checker = findradius(self.origin, 100000);

    while (checker) {
        if (checker.canSplash == 1) {
            check = pointcontents(checker.origin);

            if ((check == CONTENT_WATER) && (checker.waterSplash == 1)) {
                checker.waterSplash = 0;
                Splash(checker.origin, checker.velocity);
            }

            if (check == CONTENT_EMPTY) {
                checker.waterSplash = 1;
            }
        }

        checker = checker.chain;
    }
};
