#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID {0x71, 0x12, 0xD6, 0x24, 0x65, 0x55, 0x42, 0x79, 0x89, 0xAE, 0xC0, 0x86, 0xE9, 0x36, 0x94, 0x27}
PBL_APP_INFO(MY_UUID,
             "LCARS v1", "Slayer1551",
             2, 0, /* App major/minor version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;


BmpContainer background_image;

//BmpContainer meter_bar_image;

// TODO: Handle 12/24 mode preference when it's exposed.



const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
    RESOURCE_ID_IMAGE_DAY_NAME_SUN,
    RESOURCE_ID_IMAGE_DAY_NAME_MON,
    RESOURCE_ID_IMAGE_DAY_NAME_TUE,
    RESOURCE_ID_IMAGE_DAY_NAME_WED,
    RESOURCE_ID_IMAGE_DAY_NAME_THU,
    RESOURCE_ID_IMAGE_DAY_NAME_FRI,
    RESOURCE_ID_IMAGE_DAY_NAME_SAT
};

BmpContainer day_name_image;


const int DATENUM_IMAGE_RESOURCE_IDS[] = {
    RESOURCE_ID_IMAGE_DATENUM_0,
    RESOURCE_ID_IMAGE_DATENUM_1,
    RESOURCE_ID_IMAGE_DATENUM_2,
    RESOURCE_ID_IMAGE_DATENUM_3,
    RESOURCE_ID_IMAGE_DATENUM_4,
    RESOURCE_ID_IMAGE_DATENUM_5,
    RESOURCE_ID_IMAGE_DATENUM_6,
    RESOURCE_ID_IMAGE_DATENUM_7,
    RESOURCE_ID_IMAGE_DATENUM_8,
    RESOURCE_ID_IMAGE_DATENUM_9
};


#define TOTAL_DATE_DIGITS 2
BmpContainer date_digits_images[TOTAL_DATE_DIGITS];


const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
    RESOURCE_ID_IMAGE_NUM_0,
    RESOURCE_ID_IMAGE_NUM_1,
    RESOURCE_ID_IMAGE_NUM_2,
    RESOURCE_ID_IMAGE_NUM_3,
    RESOURCE_ID_IMAGE_NUM_4,
    RESOURCE_ID_IMAGE_NUM_5,
    RESOURCE_ID_IMAGE_NUM_6,
    RESOURCE_ID_IMAGE_NUM_7,
    RESOURCE_ID_IMAGE_NUM_8,
    RESOURCE_ID_IMAGE_NUM_9
};


#define TOTAL_TIME_DIGITS 4
BmpContainer time_digits_images[TOTAL_TIME_DIGITS];

#define TIME_ZONE_OFFSET 1
#define TOTAL_DIGITS 10
BmpContainer digits[TOTAL_DIGITS];

const int BLOCK_NUMBER[] = {
    RESOURCE_ID_IMAGE_DATENUM_0,
    RESOURCE_ID_IMAGE_DATENUM_1,
    RESOURCE_ID_IMAGE_DATENUM_2,
    RESOURCE_ID_IMAGE_DATENUM_3,
    RESOURCE_ID_IMAGE_DATENUM_4,
    RESOURCE_ID_IMAGE_DATENUM_5,
    RESOURCE_ID_IMAGE_DATENUM_6,
    RESOURCE_ID_IMAGE_DATENUM_7,
    RESOURCE_ID_IMAGE_DATENUM_8,
    RESOURCE_ID_IMAGE_DATENUM_9
};
const int ROW[] = {
    12,
    63,
    114
};

const int COLUMN[] = {
    6,
    41,
    76,
    111
};

Layer display_layer;

/* These are the variables we defined in our JavaScript earlier.  This will save
 us trying to work out where the cells should go. */
#define CELL_WIDTH 12
#define CELL_HEIGHT 2
#define CELL_PADDING_RIGHT 2
#define CELL_PADDING_TOP 2
#define CELL_OFFSET 33


/* This is a port of the same function from the JS - it returns a GRect which
 is required to draw a rectangle on the display_layer. */
GRect cell_location(int col, int row) {
    return GRect((col * (CELL_WIDTH + CELL_PADDING_RIGHT)) ,(CELL_OFFSET - (row * (CELL_HEIGHT + CELL_PADDING_TOP))), CELL_WIDTH, CELL_HEIGHT);
}

void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin) {
    
    layer_remove_from_parent(&bmp_container->layer.layer);
    bmp_deinit_container(bmp_container);
    
    bmp_init_container(resource_id, bmp_container);
    
    GRect frame = layer_get_frame(&bmp_container->layer.layer);
    frame.origin.x = origin.x;
    frame.origin.y = origin.y;
    layer_set_frame(&bmp_container->layer.layer, frame);
    
    layer_add_child(&window.layer, &bmp_container->layer.layer);
}


unsigned short get_display_hour(unsigned short hour) {
    
    if (clock_is_24h_style()) {
        return hour;
    }
    
    unsigned short display_hour = hour % 12;
    
    // Converts "0" to "12"
    return display_hour ? display_hour : 12;
    
}

int my_pow(int base, int exp) {
    int result = 1;
    for(int i=exp; i>0; i--) {
        result = result * base;
    }
    return result;
}

void update_display(PblTm *current_time) {
    // TODO: Only update changed values?
    
    set_container_image(&day_name_image, DAY_NAME_IMAGE_RESOURCE_IDS[current_time->tm_wday], GPoint(79, 3));
    
    // TODO: Remove leading zero?
    set_container_image(&date_digits_images[0], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday/10], GPoint(111, 3));
    set_container_image(&date_digits_images[1], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday%10], GPoint(118, 3));
    
    
    unsigned short display_hour = get_display_hour(current_time->tm_hour);
    
    // TODO: Remove leading zero?
    set_container_image(&time_digits_images[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(38, 30));
    set_container_image(&time_digits_images[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(62, 30));
    
    set_container_image(&time_digits_images[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(86, 30));
    set_container_image(&time_digits_images[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(110, 30));
    
    unsigned int unix_time;
    /* Convert time to seconds since epoch. */
    unix_time = ((0-TIME_ZONE_OFFSET)*3600) + /* time zone offset */
    + current_time->tm_sec /* start with seconds */
    + current_time->tm_min*60 /* add minutes */
    + current_time->tm_hour*3600 /* add hours */
    + current_time->tm_yday*86400 /* add days */
    + (current_time->tm_year-70)*31536000 /* add years since 1970 */
    + ((current_time->tm_year-69)/4)*86400 /* add a day after leap years, starting in 1973 */
    - ((current_time->tm_year-1)/100)*86400 /* remove a leap day every 100 years, starting in 2001 */
    + ((current_time->tm_year+299)/400)*86400; /* add a leap day back every 400 years, starting in 2001*/
    
    /* Draw each digit in the correct location. */
    for(int i=0; i<TOTAL_DIGITS; i++) {
        
        /* int digit_colum = i % TOTAL_COLUMNS; */
        int denominator = my_pow(10,i); /* The loop starts at the most significant digit and goes down from there. */
        int digit_value = (int)unix_time/(1000000000 / denominator); /* This gives the value for the current digit. (Casting should give us the floor of the value.) */
        unix_time = unix_time % (1000000000 / denominator); /* This subtracts the value for the current digit so that it doesn't interfere with the next iteration of the loop. */
        set_container_image(&digits[i], BLOCK_NUMBER[digit_value], GPoint(40 + (i * 9), 90)); /* Now we set this digit. */
    }
    
    
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
    
    update_display(t->tick_time);
}
void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
    /* Prevent the compiler earnings as in the display callback */
    //(void)t;
    (void)ctx;
    
    
    update_display(t->tick_time);
    
    
    
    
    
    /* Tell the application to rerender the layer */
    layer_mark_dirty(&display_layer);
}
void display_layer_update_callback(Layer *me, GContext* ctx) {
    (void)me;
    
    PblTm t;
    
    get_time(&t);
    
    unsigned short display_hour = get_display_hour(t.tm_hour);
    graphics_context_set_fill_color(ctx, GColorWhite);
    
    
    for (int cell_column_index = 0; cell_column_index < display_hour/10; cell_column_index++) {
        
        
        graphics_fill_rect(ctx, cell_location(0, cell_column_index), 0, GCornerNone);
    }
    for (int cell_column_index = 0; cell_column_index < display_hour%10; cell_column_index++) {
        
        
        graphics_fill_rect(ctx, cell_location(1, cell_column_index), 0, GCornerNone);
    }
    
    for (int cell_column_index = 0; cell_column_index < t.tm_min/10; cell_column_index++) {
        
        
        graphics_fill_rect(ctx, cell_location(2, cell_column_index), 0, GCornerNone);
    }
    for (int cell_column_index = 0; cell_column_index < t.tm_min%10; cell_column_index++) {
        
        
        graphics_fill_rect(ctx, cell_location(3, cell_column_index), 0, GCornerNone);
    }
    
    for (int cell_column_index = 0; cell_column_index < t.tm_sec/10; cell_column_index++) {
        
        
        graphics_fill_rect(ctx, cell_location(4, cell_column_index), 0, GCornerNone);
    }
    for (int cell_column_index = 0; cell_column_index < t.tm_sec%10; cell_column_index++) {
        
        
        graphics_fill_rect(ctx, cell_location(5, cell_column_index), 0, GCornerNone);
    }
    
}
void handle_init(AppContextRef ctx) {
    memset(&background_image, 0, sizeof(background_image));
    memset(&day_name_image, 0, sizeof(day_name_image));
    memset(&date_digits_images, 0, sizeof(date_digits_images));
    memset(&time_digits_images, 0, sizeof(time_digits_images));
    memset(&digits, 0, sizeof(digits));
    memset(&display_layer, 0, sizeof(display_layer));
    
    
    window_init(&window, "LCAR");
    window_stack_push(&window, true /* Animated */);
    
    resource_init_current_app(&APP_RESOURCES);
    
    bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &background_image);
    layer_init(&display_layer, GRect(43, 110, 82, 35));
    display_layer.update_proc = &display_layer_update_callback;
    
    layer_add_child(&window.layer, &background_image.layer.layer);
    
    
    layer_add_child(&window.layer, &background_image.layer.layer);
    layer_add_child(&window.layer,&display_layer);
    
 
    
    
    // Avoids a blank screen on watch start.
    PblTm tick_time;
    
    get_time(&tick_time);
    update_display(&tick_time);
    
}


void handle_deinit(AppContextRef ctx) {
    
    bmp_deinit_container(&background_image);
    
    bmp_deinit_container(&day_name_image);
    
    for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
        bmp_deinit_container(&date_digits_images[i]);
    }
    
    for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
        bmp_deinit_container(&time_digits_images[i]);
    }
    for (int i=0; i < TOTAL_DIGITS; i++) {
        bmp_deinit_container(&digits[i]);
    }
}


void pbl_main(void *params) {
    PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .deinit_handler = &handle_deinit,
        
        .tick_info = {
            .tick_handler = &handle_second_tick,
            .tick_units = SECOND_UNIT
        }
    };
    app_event_loop(params, &handlers);
}
