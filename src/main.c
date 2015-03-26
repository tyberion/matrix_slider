#include <pebble.h>

Window *my_window;
Layer *box_layer;
InverterLayer *inverter_layer;
static GFont s_time_font;
int center;

int M = 8;
int N = 18;
PropertyAnimation *animations[8] = {0};
GRect to_rect[8];
TextLayer *text_layer[8];

int offsets[8][18];
int order[8][18];

char digits[8][64];

void set_digit(int col, int num) {
    Layer *layer = text_layer_get_layer(text_layer[col]);
    
    to_rect[col] = layer_get_frame(layer);
    to_rect[col].origin.y = ((offsets[col][num]) * -21) + center - 13;
    
    if(animations[col])
        property_animation_destroy(animations[col]);
    
    animations[col] = property_animation_create_layer_frame(layer, NULL, &to_rect[col]);
    animation_set_duration((Animation*) animations[col], 1000);
    animation_schedule((Animation*) animations[col]);
}

void display_time(struct tm* tick_time) {
    int h = tick_time->tm_hour;
    int m = tick_time->tm_min;
    int d = tick_time->tm_mday;
    int mo = tick_time->tm_mon + 1;
    
    // If watch is in 12hour mode
    if(!clock_is_24h_style()) {
        if(h == 0) { //Midnight to 1am
            h = 12;
        } else if(h > 12) { //1pm to 11:59pm
            h -= 12;
        }
    }
    
    int k = 0;
    set_digit(k, d/10);
    k++;
    set_digit(k, d%10);
    k++;
    set_digit(k, h/10);
    k++;
    set_digit(k, h%10);
    k++;
    set_digit(k, m/10);
    k++;
    set_digit(k, m%10);
    k++;
    set_digit(k, mo/10);
    k++;
    set_digit(k, mo%10);
  
}

void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
    display_time(tick_time);
}

void fill_digits(int i) {
    for(int n=0; n<N; n++) {
        int p = (order[i][n] * 2);
        digits[i][p] = '0' + n;
        digits[i][p+1] = '\n';
    }
    digits[i][N * 2 + 1] = '\0';
}

void fill_offsets(int i) {
    for(int n=0; n<N; n++) {
        offsets[i][n] = order[i][n];
    }
}

void fill_order(int i) {
    for(int n=0; n<N; n++) {
        order[i][n] = n;
    }
    
    for(int n=0; n<N; ++n) {
        int k = rand() % N;
        if(n != k) {
            int tmp = order[i][k];
            order[i][k] = order[i][n];
            order[i][n] = tmp;
        }
    }
}

void handle_init(void) {
    my_window = window_create();
    window_stack_push(my_window, true);
    window_set_background_color(my_window, GColorBlack);
  
    //Create GFont
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MATRIX_21));

    Layer *root_layer = window_get_root_layer(my_window);
    GRect frame = layer_get_frame(root_layer);
  
    center = frame.size.h/2;
    
    srand(time(NULL));
    
    for(int i=0; i<M; ++i) {
        fill_order(i);
        fill_offsets(i);
        fill_digits(i);
        
        text_layer[i] = text_layer_create(GRect(i*frame.size.w/M, 0, frame.size.w/M, 800));
        text_layer_set_text_color(text_layer[i], GColorWhite);
        text_layer_set_background_color(text_layer[i], GColorClear);
        text_layer_set_font(text_layer[i], s_time_font);
        text_layer_set_text(text_layer[i], &digits[i][0]);
        text_layer_set_text_alignment(text_layer[i], GTextAlignmentCenter);
        layer_add_child(root_layer, text_layer_get_layer(text_layer[i]));
    }
    
    time_t now = time(NULL);
    struct tm *tick_time = localtime(&now);
    display_time(tick_time);
    
    tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}

void handle_deinit(void) {
    for(int i=0; i<M; ++i)
        text_layer_destroy(text_layer[i]);
    //Unload GFont
    fonts_unload_custom_font(s_time_font);
    window_destroy(my_window);
}

int main(void) {
    handle_init();
    app_event_loop();
    handle_deinit();
}
