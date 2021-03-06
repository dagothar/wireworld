#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <allegro.h>

#define VOID_COLOR 0, 0, 0
#define WIRE_COLOR 225, 175, 25
#define HEAD_COLOR 255, 255, 255
#define TAIL_COLOR 255, 227, 34
/*#define WIRE_COLOR 225, 175, 25
#define HEAD_COLOR 255, 227, 34
#define TAIL_COLOR 240, 200, 75*/

using namespace std;

const int screen_w = 800;
const int screen_h = 600;

int p_x = 0;
int p_y = 0;

enum CELL_TYPES { VOID, WIRE, E_HEAD, E_TAIL };
int colors[E_TAIL+1];

bool button_1 = false;
bool button_2 = false;

class Circuit {
      int width;
      int height;
      
      bool cells_set;
      int n_cells;
      char** cells[2];
      
      struct Display_info {
             int cell_size;
             int x_off;
             int y_off;
             BITMAP* buffer;
      } d_i;
      
      void alloc_cells();
      void setup_display();
      int get_e_heads(int x, int y);
      
      public:
             Circuit() : width(100), height(100), cells_set(0), n_cells(width * height)
                       { alloc_cells(); seed_cells(); setup_display(); };
                       
             void seed_cells();
             void power_off();
             void display();
             void draw();
             void step();
             void load_circuit(int x, int y, char* filename);
             void save_circuit(char* filename);
};

void Circuit::alloc_cells()
{
     cells[0] = new char*[width];
     cells[1] = new char*[width];
     
     for(int i = 0; i < width; ++i) {
             cells[0][i] = new char[height];
             cells[1][i] = new char[height];
     }
}

void Circuit::seed_cells()
{
     cells_set = 0;
     for(int y = 0; y < height; ++y)
             for(int x = 0; x < width; ++x) {
                     cells[0][x][y] = 0;
                     cells[1][x][y] = cells[0][x][y];
             }
}

void Circuit::power_off()
{
     for(int y = 0; y < height; ++y)
             for(int x = 0; x < width; ++x)
                     if((cells[cells_set][x][y] == E_HEAD) || (cells[!cells_set][x][y] == E_HEAD)) {
                                               cells[cells_set][x][y] = WIRE;
                                               cells[!cells_set][x][y] = WIRE;
                     }
}

void Circuit::setup_display()
{
     int cell_dx = screen_w / width;
     int cell_dy = screen_h / height;
     d_i.cell_size = cell_dx < cell_dy ? cell_dx : cell_dy;
     d_i.x_off = (screen_w - d_i.cell_size * width) / 2;
     d_i.y_off = (screen_h - d_i.cell_size * height) / 2;
     d_i.buffer = create_bitmap(d_i.cell_size * width, d_i.cell_size * height);
     colors[VOID] = makecol(VOID_COLOR);
     colors[WIRE] = makecol(WIRE_COLOR);
     colors[E_HEAD] = makecol(HEAD_COLOR);
     colors[E_TAIL] = makecol(TAIL_COLOR);
}

int Circuit::get_e_heads(int x, int y)
{
    int heads = 0;
    for(int _y = y-1; _y <= y+1; ++_y)
            for(int _x = x-1; _x <= x+1; ++_x)
                    if((_x>=0) && (_x<width) && (_y>=0) && (_y<height)) if(cells[!cells_set][_x][_y] == E_HEAD) ++heads;
    return heads;
}

void Circuit::display()
{
     //scare_mouse();
     clear(d_i.buffer);
     int color = makecol(255, 255, 0);
     
     rect(screen, d_i.x_off-1, d_i.y_off-1, d_i.x_off + width * d_i.cell_size+1, d_i.y_off + height * d_i.cell_size+1, color);
     
     for(int y = 0; y < height; ++y)
             for(int x = 0; x < width; ++x) {
                     switch(cells[cells_set][x][y]) {
                                                    case VOID:
                                                         color = makecol(VOID_COLOR);
                                                         break;
                                                    case WIRE:
                                                         color = makecol(WIRE_COLOR);
                                                         break;
                                                    case E_HEAD:
                                                         color = makecol(HEAD_COLOR);
                                                         break;
                                                    case E_TAIL:
                                                         color = makecol(TAIL_COLOR);
                                                         break;
                     }
                     rectfill(d_i.buffer, x * d_i.cell_size + 1, y * d_i.cell_size + 1, (x+1) * d_i.cell_size - 1, (y+1) * d_i.cell_size - 1, color);
             }

     blit(d_i.buffer, screen, 0, 0, d_i.x_off, d_i.y_off, d_i.buffer->w, d_i.buffer->h);
     unscare_mouse();
}

void Circuit::draw()
{
     int oldx = 0;
     int oldy = 0;
     int x, y, z = mouse_z;
     char current_brush = WIRE;
//	show_mouse(screen);
     
     bool end_draw = false;
     do {
         if((mouse_b & 1) && (!button_1)) {
                     button_1 = true;
                     x = (mouse_x - d_i.x_off) / d_i.cell_size;
                     y = (mouse_y - d_i.y_off) / d_i.cell_size;
                     if((x>=0) && (x<width) && (y>=0) && (y<height))
                               cells[cells_set][x][y] = current_brush;
         }
         if(!(mouse_b & 1) || (oldx != x) || (oldy != y)) button_1 = false;
         
         if((mouse_b & 2) && (!button_2)) {
                     button_2 = true;
                     x = (mouse_x - d_i.x_off) / d_i.cell_size;
                     y = (mouse_y - d_i.y_off) / d_i.cell_size;
                     if((x>=0) && (x<width) && (y>=0) && (y<height))
                               cells[cells_set][x][y] = VOID;
         }
         if(!(mouse_b & 2) || (oldx != x) || (oldy != y)) button_2 = false;
         
         if(mouse_z != z)
                    if(mouse_z != z) {
                               ++current_brush;
                               if(current_brush > E_TAIL) current_brush = WIRE;
                               z = mouse_z;
                    } else
                    if(mouse_z != z ) {
                               --current_brush;
                               if(current_brush < WIRE) current_brush = E_TAIL;
                               z = mouse_z;
                    }
	scare_mouse();
         rectfill(screen, 0, 0, 10, 10, colors[current_brush]);
	unscare_mouse();
         
         if(keypressed()) {
                          int key = readkey();
                          if((key & 0xff) >= '0' && (key & 0xff) <= '9') {
                                 char* circuit;
                                 switch(key & 0xff) {
                                             case '0':
                                                  seed_cells();
                                                  continue;
                                             case '1':
                                                  circuit = "clock_6.wir";
                                                  break;
                                             case '2':
                                                  circuit = "clock_12.wir";
                                                  break;
                                             case '3':
                                                  circuit = "or_gate.wir";
                                                  break;
                                             case '4':
                                                  circuit = "xor_gate.wir";
                                                  break;
                                             case '5':
                                                  circuit = "freq_dup.wir";
                                                  break;
                                             case '6':
                                                  circuit = "and_gate.wir";
                                                  break;
                                             default:
                                                     circuit = "1_wire.wir";
                                                     break;
                                 }
                                 while(!(mouse_b & 1)); mouse_b ^= 1;
                                 x = (mouse_x - d_i.x_off) / d_i.cell_size;
                                 y = (mouse_y - d_i.y_off) / d_i.cell_size;
                                 load_circuit(x, y, circuit);
                                 clear_keybuf();
                          } else
                          switch(key >> 8) {
                                      case KEY_S:
                                           save_circuit("save.wir");
                                           break;
                                      case KEY_L:
                                           load_circuit(0, 0, "save.wir");
                                           break;
                                      case KEY_SPACE:
                                      case KEY_ESC:
                                           end_draw = true;
                                           break;
                          }
         }
         
         display();
     } while(!end_draw);
     
     clear_keybuf();
}

void Circuit::step()
{
     cells_set = !cells_set;
     
     for(int y = 0; y < height; ++y)
             for(int x = 0; x < width; ++x) {
                     if(cells[!cells_set][x][y] == VOID) cells[cells_set][x][y] = VOID;
                     if(cells[!cells_set][x][y] == E_TAIL) cells[cells_set][x][y] = WIRE;
                     if(cells[!cells_set][x][y] == E_HEAD) cells[cells_set][x][y] = E_TAIL;
                     if(cells[!cells_set][x][y] == WIRE) {
                                                int heads = get_e_heads(x, y);
                                                if((heads == 1) || (heads == 2))
                                                          cells[cells_set][x][y] = E_HEAD;
                                                else
                                                    cells[cells_set][x][y] = WIRE;
                     }
             }
}

void Circuit::load_circuit(int x, int y, char* filename)
{
     ifstream source(filename);
     
     int w, h;
     char c;
     source >> w >> h; source.ignore();
     for(int dy = y; dy < y + h; ++dy) {
             for(int dx = x; dx < x + w; ++dx) {
                     source.get(c);
                     if((dx < width-1) && (dy < height-1))
                            switch(c) {
                                      case '0':
                                           cells[cells_set][dx][dy] = VOID;
                                           break;
                                      case '1':
                                           cells[cells_set][dx][dy] = WIRE;
                                           break;
                                      case '2':
                                           cells[cells_set][dx][dy] = E_HEAD;
                                           break;
                                      case '3':
                                           cells[cells_set][dx][dy] = E_TAIL;
                                           break;
					default:
						--dx;
						break;
                            }
             }
             source.ignore();
     }
     source.close();
}

void Circuit::save_circuit(char* filename)
{
     ofstream save(filename);
     save << width << ' ' << height << endl;
     for(int y = 0; y < height; ++y) {
             for(int x = 0; x < width; ++x)
                     switch(cells[cells_set][x][y]) {
                                                    case VOID:
                                                         save << '0'; break;
                                                    case WIRE:
                                                         save << '1'; break;
                                                    case E_HEAD:
                                                         save << '2'; break;
                                                    case E_TAIL:
                                                         save << '3'; break;
                     }
             save << endl;
     }
     save.close();
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    allegro_init();
    install_keyboard();
    install_mouse();
    
    set_color_depth(32);
    bool windowed = true;
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, screen_w, screen_h, 0, 0);
    
    set_mouse_sprite_focus(0, 0);
    show_mouse(screen);
    
    Circuit a;
    a.load_circuit(0, 0, "save.wir");
    a.display();
    a.draw();
    int delay = 100;
    
    bool quit = false;
    while(!quit) {
                         a.step();
                         a.display();
                         rest(delay);
                         //if((mouse_b & 1) || (mouse_b & 2)) { a.draw(); button_1 = true; button_2 = true; }
                         if(keypressed()) {
                                          int key = readkey() >> 8;
                                          clear_keybuf();
                                          switch(key) {
                                                      case KEY_ESC:
                                                           quit = true;
                                                           break;
                                                      case KEY_ENTER:
                                                           if(windowed) {
                                                                       set_gfx_mode(GFX_AUTODETECT, screen_w, screen_h, 0, 0);
                                                                       windowed = false;
                                                           } else {
                                                                  set_gfx_mode(GFX_AUTODETECT_WINDOWED, screen_w, screen_h, 0, 0);
                                                                  windowed = true;
                                                           }
                                                           a.display();
                                                           break;
                                                      case KEY_SPACE:
                                                           a.draw();
                                                           break;
                                                      case KEY_0_PAD:
                                                           a.seed_cells();
                                                           break;
                                                      case KEY_ENTER_PAD:
                                                           a.power_off();
                                                           break;
                                                      case KEY_S:
                                                           a.save_circuit("save.wir");
                                                           break;
                                                      case KEY_L:
                                                           a.load_circuit(0, 0, "save.wir");
                                                           break;
                                                      case KEY_PLUS_PAD:
                                                           if(delay > 10) delay -= 10;
                                                           break;
                                                      case KEY_MINUS_PAD:
                                                           if(delay < 200) delay += 10;
                                                           break;
                                          }
                         }
    }
    
    return EXIT_SUCCESS;
}
END_OF_MAIN();
