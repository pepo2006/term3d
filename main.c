#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>

#define WIDTH 40
#define HEIGHT 40

#define PI 3.14159

void clear_screen(void){ printf("\x1b[H\x1b[J"); }

static inline void swap(int *a, int *b){
    int tmp = *a;
    *a = *b;
    *b = tmp; 
}

static inline float abs_f(float a){ return (a > 0) ? a : -a; }

void print_buff(char *buff, size_t size){
    for(size_t i = 0; i < size; ++i){
        putc(*(buff+i), stdout);
        putc(' ', stdout);
        if(!((i+1)%WIDTH)) putc('\n', stdout);
    }
}

static inline void put_p2(char *buff, int x, int y, char c){
    x = x%WIDTH;
    y = y%HEIGHT;
    *(buff+x+(HEIGHT-y-1)*WIDTH) = c;
}
 
void put_line2(char *buff, int x1, int y1, int x2, int y2, char c){
    if(x2 == x1){
        if(y1 > y2){
            size_t tmp = y1;
            y1 = y2; y2 = tmp;
        }
        for(int i = y1; i <= y2; i++){
            put_p2(buff, x1, i, c);
        }
        return;
    }

    if(x1 > x2){
        swap(&x1, &x2);
        swap(&y1, &y2);
    }

    float mx = ((float)(y2-y1))/(float)(x2-x1);

    if(abs_f(mx) <= 1){
        float h = y1-mx*x1;
        for(int i = x1; i <= x2; i++){
            put_p2(buff, i, (int)(mx*i+h), c);
        }
        return;
    }

    if(y1 > y2){
        swap(&y1, &y2);
        swap(&x1, &x2);
    }

    float my = ((float)(x2-x1))/(float)(y2-y1);
    
    if(abs_f(my) <= 1){
        float h = x1-my*y1;
        for(int i = y1; i <= y2; i++){
            put_p2(buff, (int)(my*i+h), i, c);
        }
    }
}

typedef struct {
    size_t x, y;
} vec2_t;

typedef struct {
    size_t fst, snd;
} pair_t;

typedef struct {
    float x, y, z;
} vec3_f;

void sum_vec3(vec3_f v, vec3_f u, vec3_f *r){
    r->x = v.x + u.x; 
    r->y = v.y + u.y; 
    r->z = v.z + u.z;
}

vec3_f scale_vec3(vec3_f v, float a){
    vec3_f r = { a*v.x, a*v.y, a*v.z };
    return r;
}

vec3_f rotate_p3(vec3_f v, vec3_f ang){
    float cx = cos(ang.x);
    float cy = cos(ang.y);
    float cz = cos(ang.z);
    float sx = sin(ang.x);
    float sy = sin(ang.y);
    float sz = sin(ang.z);

    vec3_f r;

    r.x = cy*(sz*v.y+cz*v.x)-sy*v.z;
    r.y = sx*(cy*v.z+sy*(sz*v.y+cz*v.x))+cx*(cz*v.y-sz*v.x);
    r.z = cx*(cy*v.z+sy*(sz*v.y+cz*v.x))-sx*(cz*v.y-sz*v.x);

    return r;
}

vec2_t project_p3(vec3_f p, vec3_f c, vec3_f c_a, vec3_f e){
    vec3_f tor = { p.x - c.x, p.y - c.y, p.z - c.z };
    vec3_f d = rotate_p3(tor, c_a);

    vec2_t b = {
        .x = (size_t)(WIDTH/2+e.z*d.x/d.z+e.x),
        .y = (size_t)(HEIGHT/2+e.z*d.y/d.z+e.y)
    };

    return b;
}

#if 1

int main(){
    struct termios old_settings, settings;

    tcgetattr(STDIN_FILENO, &old_settings);
    settings = old_settings;

    settings.c_lflag &= (~ICANON & ~ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &settings);

    

    clear_screen();

    size_t screen_size = WIDTH*HEIGHT;
    char *screen = malloc(screen_size*sizeof(char));
    memset(screen, ' ', screen_size*sizeof(char));

    char c = '#';

    //1   3
    // 5 7
    // 4 6
    //0   2

    vec3_f camera          = { 0, 0, -30 };
    vec3_f angles          = { 0, 0,  0  };
    vec3_f display_surface = { 0, 0,  20 };

    vec3_f vertices[] = {{ -10, -10,  10 }, { -10,  10,  10 },
                         {  10, -10,  10 }, {  10,  10,  10 },
                         { -10, -10, -10 }, { -10,  10, -10 },
                         {  10, -10, -10 }, {  10,  10, -10 }};

    pair_t edges[] = {{ 0, 1 }, { 3, 1 }, { 5, 1 }, { 6, 2 }, 
                      { 0, 2 }, { 3, 2 }, { 5, 4 }, { 6, 4 }, 
                      { 0, 4 }, { 3, 7 }, { 5, 7 }, { 6, 7 }}; 
//    float ang_x = PI/32,
//          ang_y = PI/32,
//          ang_z = PI/32;

    for(;;){
//        angles.y -= PI/32;
//        angles.x -= PI/32;
//        angles.z -= PI/32;

//        if(angles.y < 2*PI) angles.y += 2*PI;
//        if(angles.x < 2*PI) angles.x += 2*PI;
//        if(angles.z < 2*PI) angles.z += 2*PI;

        vec2_t vertices_p[8]; 

        for(int i = 0; i < 8; i++){
            vertices_p[i] = project_p3(vertices[i], camera, angles, display_surface);
        }

        for(int i = 0; i < 12; i++){
            put_line2(screen, vertices_p[edges[i].fst].x,
                              vertices_p[edges[i].fst].y,
                              vertices_p[edges[i].snd].x,
                              vertices_p[edges[i].snd].y, c);
        }

        clear_screen();
        print_buff(screen, screen_size);
        fflush(stdout);
        memset(screen, ' ', screen_size);

        vec3_f camera_local = { 0, 0, 0 };

        switch(getc(stdin)){
            case 'w':
                camera_local.z =  1;
                break;
            case 'a':
                camera_local.x = -1;
                break;
            case 's':
                camera_local.z  = -1;
                break;
            case 'd':
                camera_local.x  =  1;
                break;
            case '\x1b': 
                getc(stdin);
                switch(getc(stdin)){
                    case 'A':
                        angles.x -= PI/32;
                        break;
                    case 'B':
                        angles.x += PI/32;
                        break;
                    case 'C':
                        angles.y += PI/32;
                        break;
                    case 'D':
                        angles.y -= PI/32;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        sum_vec3(camera, rotate_p3(camera_local, scale_vec3(angles, -1)), &camera);

    }

    tcgetattr(STDIN_FILENO, &old_settings);

    return 0;
}

// WIDTH 40
// HEIGHT 40

#else

int main(void){
    size_t screen_size = WIDTH*HEIGHT;
    char *screen = malloc(screen_size*sizeof(char));
    memset(screen, ' ', screen_size*sizeof(char));

    put_line2(screen, 10, 10, 50, 30, '0');
    clear_screen();
    print_buff(screen, screen_size);
    fflush(stdout);
    memset(screen, ' ', screen_size);

    return 0;
}
#endif
