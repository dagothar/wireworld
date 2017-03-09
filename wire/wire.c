#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>
#include <time.h>

int cir_w, cir_h;
BITMAP *bmp1, *bmp2;
PALETTE pal;
int wire, head, tail;
BITMAP *circuit, *mirror;
long generations = 0, lgen = 0;
int speed = 0;
clock_t t0;
int startx = 0, starty = 1;
int endx = 0, endy = 0;

int get(BITMAP *bmp, int x, int y)
{
	return getpixel(bmp, x, y);
	//return ((long *)bmp->line[y])[x];
}

void put(BITMAP *bmp, int x, int y, int color)
{
	putpixel(bmp, x, y, color);
	//((long *)bmp->line[y])[x] = color;
}

void go(void)
{
	int x, y;
	int i, j;
	int n;
	for(y = starty; y < endy; ++y) {
		for(x = startx; x < endx; ++x) {
			int cell = get(circuit, x, y); //((long *)circuit->line[y])[x];
			if(!cell) continue;
			if(cell == head) {
				put(mirror, x, y, tail); //((long *)mirror->line[y])[x] = tail;
			}
			else if(cell == tail) {
				put(mirror, x, y, wire); //((long *)mirror->line[y])[x] = wire;
			}
			else if(cell == wire) {
				n = 0;
				for(i = y-1; i <= y+1 && i >= 0 && i < endy; ++i)
					for(j = x-1; j <= x+1 && j >= 0 && j < endx; ++j)
						//if(((long *)circuit->line[i])[j] == head) ++n;
            if (get(circuit, j, i) == head) ++n;
				if(n > 0 && n < 3) put(mirror, x, y, head); //((long *)mirror->line[y])[x] = head;
				else put(mirror, x, y, cell); //((long *)mirror->line[y])[x] = cell;
			}
		}
  }

	BITMAP *tmp = mirror;
	mirror = circuit;
	circuit = tmp;
	++generations;
	if(clock() - t0 > CLOCKS_PER_SEC) {
		speed = generations - lgen;
		lgen = generations;
		t0 = clock();
	}
}

int main(int argc, char *argv[])
{
	t0 = clock();
	if(argc < 1) return -1;

	char *name;
	if(argc > 2) name = argv[2];
	else name = "circuit.bmp";

	if(argc == 7) {
		startx = atoi(argv[3]);
		starty = atoi(argv[4]);
		endx = atoi(argv[5]);
		endy = atoi(argv[6]);
	}

	allegro_init();
	install_keyboard();
	install_timer();

	set_color_depth(32);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 800, 600, 0, 0);

	//wire = makecol(225, 175, 25);
	//head = makecol(255, 255, 255);
	//tail = makecol(255, 227, 34);

	bmp1 = load_bitmap(argv[1], pal);
	if(!bmp1) {
		allegro_exit();
		return -1;
	}

	cir_w = bmp1->w;
	cir_h = bmp1->h;
	if(endx == 0) endx = cir_w;
	if(endy == 0) endy = cir_h; 

	wire = get(bmp1, 0, 0);
	head = get(bmp1, 1, 0);
	tail = get(bmp1, 2, 0);
	
	bmp2 = create_bitmap(cir_w, cir_h);
	clear_bitmap(bmp2);
	blit(bmp1, bmp2, 0, 0, 0, 0, cir_w, cir_h);

	circuit = bmp1;
	mirror = bmp2;

	stretch_blit(circuit, screen, 0, 0, cir_w, cir_h, 0, 0, SCREEN_W, SCREEN_H);

	char run = 1;
	char show = 1;
	char text = 0;
	char pause = 0;
	char autosave = 0;
	while(run) {
		if(keypressed()) {
			if(key[KEY_S]) save_bitmap(name, circuit, pal);
			if(key[KEY_V]) show = !show;
			if(key[KEY_A]) autosave = !autosave;
			if(key[KEY_T]) text = !text;
			if(key[KEY_SPACE]) pause = !pause;
			if(key[KEY_Q]) run = 0;
			clear_keybuf();
		}

		if(autosave && !(generations % 10000)) save_bitmap(name, circuit, pal);

		if(text) {
			//rectfill(screen, 0, 0, 100, 10, makecol(0, 0, 0));
			textprintf_ex(screen, font, 0, 0, head, 0, "Gen: %d Speed: %d sec/gen: %.3lf", generations, speed, 1.0 / speed);
		}
		if(!pause) go();
		if(show) stretch_blit(circuit, screen, 0, 0, cir_w, cir_h, 0, 0, SCREEN_W, SCREEN_H);
		//rest(0);
	}

	allegro_exit();
	return 0;
}
