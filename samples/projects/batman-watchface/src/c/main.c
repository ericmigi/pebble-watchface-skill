#include <pebble.h>

// ============================================================================
// BATMAN/BAT SIGNAL WATCHFACE
// Animated searchlight sweeping across Gotham's night sky
// ============================================================================

// Animation timing
#define ANIMATION_INTERVAL 50
#define ANIMATION_INTERVAL_LOW_POWER 100
#define LOW_BATTERY_THRESHOLD 20

// Searchlight parameters
#define BEAM_SWEEP_LEFT_DEG -55
#define BEAM_SWEEP_RIGHT_DEG 55
#define BEAM_SPEED 250

// Bat symbol
#define BAT_SIZE 22

// Stars
#define MAX_STARS 15

// Helper macro
#define DEG_TO_TRIG(deg) ((deg) * TRIG_MAX_ANGLE / 360)

// ============================================================================
// DATA STRUCTURES
// ============================================================================

typedef struct {
    int32_t beam_angle;
    int beam_direction;
    int pulse_intensity;
    bool bat_illuminated;
} SearchlightState;

typedef struct {
    GPoint center;
    int base_size;
    int glow_radius;
    bool visible;
} BatSymbolState;

typedef struct {
    GPoint pos;
    int8_t brightness;
    int8_t twinkle_phase;
    int8_t twinkle_speed;
} Star;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static AppTimer *s_animation_timer;

static SearchlightState s_searchlight;
static BatSymbolState s_bat_symbol;
static Star s_stars[MAX_STARS];

static int s_battery_level = 100;
static bool s_is_charging = false;
static int s_animation_interval = ANIMATION_INTERVAL;

// Screen dimensions
static int16_t s_screen_width;
static int16_t s_screen_height;
static int16_t s_center_x;
static int16_t s_skyline_top;

// Time buffer
static char s_time_buffer[8];
static char s_date_buffer[16];

// Bat symbol GPath - Classic Batman logo shape (larger, cleaner)
static GPath *s_bat_path = NULL;
static GPoint s_bat_points[] = {
    // Start at top center, go clockwise
    {0, -6},       // Top of head
    {5, -14},      // Right ear tip
    {6, -4},       // Right ear base
    {12, -2},      // Right inner wing
    {30, -8},      // Right wing tip (far out and up)
    {22, 2},       // Right wing curve in
    {26, 8},       // Right wing lower point
    {16, 6},       // Right wing scallop
    {10, 14},      // Right wing bottom
    {0, 8},        // Bottom center tail
    {-10, 14},     // Left wing bottom
    {-16, 6},      // Left wing scallop
    {-26, 8},      // Left wing lower point
    {-22, 2},      // Left wing curve in
    {-30, -8},     // Left wing tip (far out and up)
    {-12, -2},     // Left inner wing
    {-6, -4},      // Left ear base
    {-5, -14},     // Left ear tip
};
static GPathInfo s_bat_path_info = {
    .num_points = 18,
    .points = s_bat_points
};

// ============================================================================
// COLOR DEFINITIONS
// ============================================================================

#ifdef PBL_COLOR
    #define COLOR_SKY GColorOxfordBlue
    #define COLOR_SKY_GLOW GColorDukeBlue
    #define COLOR_BEAM_BRIGHT GColorYellow
    #define COLOR_BEAM_MID GColorRajah
    #define COLOR_BEAM_DIM GColorWindsorTan
    #define COLOR_BAT_SYMBOL GColorBlack
    #define COLOR_BAT_GLOW GColorYellow
    #define COLOR_BAT_RING GColorChromeYellow
    #define COLOR_SKYLINE GColorDarkGray
    #define COLOR_SKYLINE_DARK GColorBlack
    #define COLOR_WINDOWS GColorYellow
    #define COLOR_TIME GColorWhite
    #define COLOR_DATE GColorLightGray
    #define COLOR_STAR GColorWhite
    #define COLOR_STAR_DIM GColorLightGray
    #define COLOR_BATTERY_GOOD GColorGreen
    #define COLOR_BATTERY_MED GColorOrange
    #define COLOR_BATTERY_LOW GColorRed
#else
    #define COLOR_SKY GColorBlack
    #define COLOR_SKY_GLOW GColorBlack
    #define COLOR_BEAM_BRIGHT GColorWhite
    #define COLOR_BEAM_MID GColorLightGray
    #define COLOR_BEAM_DIM GColorDarkGray
    #define COLOR_BAT_SYMBOL GColorBlack
    #define COLOR_BAT_GLOW GColorWhite
    #define COLOR_BAT_RING GColorWhite
    #define COLOR_SKYLINE GColorDarkGray
    #define COLOR_SKYLINE_DARK GColorBlack
    #define COLOR_WINDOWS GColorWhite
    #define COLOR_TIME GColorWhite
    #define COLOR_DATE GColorLightGray
    #define COLOR_STAR GColorWhite
    #define COLOR_STAR_DIM GColorLightGray
    #define COLOR_BATTERY_GOOD GColorWhite
    #define COLOR_BATTERY_MED GColorWhite
    #define COLOR_BATTERY_LOW GColorWhite
#endif

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

static void init_stars(void) {
    // Seed random from time
    srand(time(NULL));

    for (int i = 0; i < MAX_STARS; i++) {
        // Random positions in sky area (not in skyline)
        s_stars[i].pos.x = rand() % s_screen_width;
        s_stars[i].pos.y = 10 + (rand() % (s_skyline_top - 30));
        s_stars[i].brightness = rand() % 4;
        s_stars[i].twinkle_phase = rand() % 30;
        s_stars[i].twinkle_speed = 1 + (rand() % 3);
    }
}

static void init_searchlight(void) {
    s_searchlight.beam_angle = 0;
    s_searchlight.beam_direction = 1;
    s_searchlight.pulse_intensity = 0;
    s_searchlight.bat_illuminated = false;
}

static void init_bat_symbol(void) {
    #ifdef PBL_ROUND
    s_bat_symbol.center = GPoint(s_center_x, 85);
    #else
    s_bat_symbol.center = GPoint(s_center_x, 80);
    #endif
    s_bat_symbol.base_size = BAT_SIZE;
    s_bat_symbol.glow_radius = 0;
    s_bat_symbol.visible = false;
}

// ============================================================================
// ANIMATION UPDATE FUNCTIONS
// ============================================================================

static void update_searchlight(void) {
    // Sweep beam back and forth
    s_searchlight.beam_angle += s_searchlight.beam_direction * BEAM_SPEED;

    int32_t left_limit = DEG_TO_TRIG(BEAM_SWEEP_LEFT_DEG);
    int32_t right_limit = DEG_TO_TRIG(BEAM_SWEEP_RIGHT_DEG);

    if (s_searchlight.beam_angle >= right_limit) {
        s_searchlight.beam_angle = right_limit;
        s_searchlight.beam_direction = -1;
    } else if (s_searchlight.beam_angle <= left_limit) {
        s_searchlight.beam_angle = left_limit;
        s_searchlight.beam_direction = 1;
    }

    // Check if beam illuminates bat (bat is at angle 0, straight up)
    int32_t angle_diff = s_searchlight.beam_angle;
    if (angle_diff < 0) angle_diff = -angle_diff;

    int32_t illumination_threshold = DEG_TO_TRIG(18);

    if (angle_diff < illumination_threshold) {
        s_searchlight.bat_illuminated = true;
        int proximity = ((illumination_threshold - angle_diff) * 100) / illumination_threshold;
        if (proximity > s_searchlight.pulse_intensity) {
            s_searchlight.pulse_intensity = proximity;
        }
    } else {
        s_searchlight.bat_illuminated = false;
        if (s_searchlight.pulse_intensity > 0) {
            s_searchlight.pulse_intensity -= 4;
            if (s_searchlight.pulse_intensity < 0) s_searchlight.pulse_intensity = 0;
        }
    }
}

static void update_bat_symbol(void) {
    int target_glow = (s_searchlight.pulse_intensity * 12) / 100;

    if (s_bat_symbol.glow_radius < target_glow) {
        s_bat_symbol.glow_radius += 2;
        if (s_bat_symbol.glow_radius > target_glow) {
            s_bat_symbol.glow_radius = target_glow;
        }
    } else if (s_bat_symbol.glow_radius > target_glow) {
        s_bat_symbol.glow_radius--;
    }

    s_bat_symbol.visible = (s_searchlight.pulse_intensity > 5);
}

static void update_stars(void) {
    for (int i = 0; i < MAX_STARS; i++) {
        s_stars[i].twinkle_phase = (s_stars[i].twinkle_phase + s_stars[i].twinkle_speed) % 40;

        if (s_stars[i].twinkle_phase < 10) {
            s_stars[i].brightness = s_stars[i].twinkle_phase / 3;
        } else if (s_stars[i].twinkle_phase < 20) {
            s_stars[i].brightness = 3;
        } else if (s_stars[i].twinkle_phase < 30) {
            s_stars[i].brightness = (30 - s_stars[i].twinkle_phase) / 3;
        } else {
            s_stars[i].brightness = 0;
        }
    }
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

static void draw_sky(GContext *ctx, GRect bounds) {
    graphics_context_set_fill_color(ctx, COLOR_SKY);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    #ifdef PBL_COLOR
    // City glow at bottom
    GRect glow_rect = GRect(0, s_skyline_top - 20, bounds.size.w, 25);
    graphics_context_set_fill_color(ctx, COLOR_SKY_GLOW);
    graphics_fill_rect(ctx, glow_rect, 0, GCornerNone);
    #endif
}

static void draw_stars(GContext *ctx) {
    for (int i = 0; i < MAX_STARS; i++) {
        if (s_stars[i].brightness > 0) {
            #ifdef PBL_COLOR
            GColor star_color = (s_stars[i].brightness >= 2) ? COLOR_STAR : COLOR_STAR_DIM;
            #else
            GColor star_color = COLOR_STAR;
            #endif

            graphics_context_set_stroke_color(ctx, star_color);
            graphics_draw_pixel(ctx, s_stars[i].pos);

            if (s_stars[i].brightness >= 3) {
                graphics_draw_pixel(ctx, GPoint(s_stars[i].pos.x - 1, s_stars[i].pos.y));
                graphics_draw_pixel(ctx, GPoint(s_stars[i].pos.x + 1, s_stars[i].pos.y));
                graphics_draw_pixel(ctx, GPoint(s_stars[i].pos.x, s_stars[i].pos.y - 1));
                graphics_draw_pixel(ctx, GPoint(s_stars[i].pos.x, s_stars[i].pos.y + 1));
            }
        }
    }
}

static void draw_searchlight_beam(GContext *ctx) {
    GPoint origin = GPoint(s_center_x, s_screen_height + 30);
    int beam_length = s_screen_height + 50;

    // Calculate beam direction
    int16_t dx = (sin_lookup(s_searchlight.beam_angle) * beam_length) / TRIG_MAX_RATIO;
    int16_t dy = -(cos_lookup(s_searchlight.beam_angle) * beam_length) / TRIG_MAX_RATIO;

    GPoint beam_end = GPoint(origin.x + dx, origin.y + dy);

    // Draw beam with varying widths for glow effect
    #ifdef PBL_COLOR
    // Outer glow (widest, dimmest)
    graphics_context_set_stroke_color(ctx, COLOR_BEAM_DIM);
    graphics_context_set_stroke_width(ctx, 35);
    graphics_draw_line(ctx, origin, beam_end);

    // Middle beam
    graphics_context_set_stroke_color(ctx, COLOR_BEAM_MID);
    graphics_context_set_stroke_width(ctx, 20);
    graphics_draw_line(ctx, origin, beam_end);

    // Core beam (brightest)
    graphics_context_set_stroke_color(ctx, COLOR_BEAM_BRIGHT);
    graphics_context_set_stroke_width(ctx, 8);
    graphics_draw_line(ctx, origin, beam_end);
    #else
    // B&W: Simple beam with lines for texture
    graphics_context_set_stroke_color(ctx, COLOR_BEAM_BRIGHT);
    graphics_context_set_stroke_width(ctx, 25);
    graphics_draw_line(ctx, origin, beam_end);

    // Add dark stripes for texture
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 1);
    for (int i = 0; i < 8; i++) {
        int offset = -12 + (i * 3);
        int32_t perp_angle = s_searchlight.beam_angle + TRIG_MAX_ANGLE / 4;
        int16_t ox = (sin_lookup(perp_angle) * offset) / TRIG_MAX_RATIO;
        int16_t oy = -(cos_lookup(perp_angle) * offset) / TRIG_MAX_RATIO;
        graphics_draw_line(ctx,
            GPoint(origin.x + ox, origin.y + oy),
            GPoint(beam_end.x + ox, beam_end.y + oy));
    }
    #endif
}

static void draw_bat_symbol(GContext *ctx) {
    if (!s_bat_symbol.visible && s_bat_symbol.glow_radius == 0) {
        return;
    }

    int cx = s_bat_symbol.center.x;
    int cy = s_bat_symbol.center.y;

    // Draw glow circle
    if (s_bat_symbol.glow_radius > 0) {
        #ifdef PBL_COLOR
        graphics_context_set_fill_color(ctx, COLOR_BEAM_DIM);
        graphics_fill_circle(ctx, s_bat_symbol.center, 45 + s_bat_symbol.glow_radius);
        graphics_context_set_fill_color(ctx, COLOR_BEAM_MID);
        graphics_fill_circle(ctx, s_bat_symbol.center, 40 + s_bat_symbol.glow_radius);
        graphics_context_set_fill_color(ctx, COLOR_BAT_GLOW);
        graphics_fill_circle(ctx, s_bat_symbol.center, 35 + s_bat_symbol.glow_radius);
        #else
        graphics_context_set_fill_color(ctx, COLOR_BAT_GLOW);
        graphics_fill_circle(ctx, s_bat_symbol.center, 35 + s_bat_symbol.glow_radius);
        #endif
    }

    // Draw SLEEK MODERN BATMAN LOGO - Arkham style
    graphics_context_set_fill_color(ctx, COLOR_BAT_SYMBOL);

    // 20-point Batman logo - smooth wing curves, proper proportions
    GPoint bat_logo[] = {
        // LEFT WING - tip is LOWEST, curves UP to shoulder peak
        {cx - 55, cy + 9},     // 0: Left wing TIP (lowest, angled down-left)
        {cx - 48, cy + 2},     // 1: Left wing lower curve
        {cx - 40, cy - 4},     // 2: Left wing mid curve
        {cx - 30, cy - 9},     // 3: Left shoulder peak (HIGHEST wing point)
        {cx - 20, cy - 6},     // 4: Inner wing
        {cx - 10, cy - 7},     // 5: Left of head
        {cx - 5,  cy - 18},    // 6: Left ear tip
        {cx,      cy - 8},     // 7: Between ears (dip)
        {cx + 5,  cy - 18},    // 8: Right ear tip
        {cx + 10, cy - 7},     // 9: Right of head
        {cx + 20, cy - 6},     // 10: Inner wing
        {cx + 30, cy - 9},     // 11: Right shoulder peak (HIGHEST wing point)
        {cx + 40, cy - 4},     // 12: Right wing mid curve
        {cx + 48, cy + 2},     // 13: Right wing lower curve
        {cx + 55, cy + 9},     // 14: Right wing TIP (lowest, angled down-right)
        // BOTTOM EDGE - shallow inward curve
        {cx + 32, cy + 5},     // 15: Right lower scallop
        {cx + 15, cy + 2},     // 16: Right inner
        {cx,      cy + 5},     // 17: Bottom center (shallow)
        {cx - 15, cy + 2},     // 18: Left inner
        {cx - 32, cy + 5},     // 19: Left lower scallop
    };

    GPathInfo bat_info = { .num_points = 20, .points = bat_logo };
    GPath *bat = gpath_create(&bat_info);
    gpath_draw_filled(ctx, bat);
    gpath_destroy(bat);
}

static void draw_skyline(GContext *ctx) {
    // Gotham skyline silhouette
    int sy = s_skyline_top;

    // Building definitions: x_start, x_end, height_above_skyline_top
    struct { int16_t x1, x2, h; } buildings[] = {
        {0, 12, 8},       // Left building
        {10, 22, 25},     // Tall tower 1
        {20, 32, 15},     // Medium building
        {30, 45, 35},     // Wayne Tower (tallest)
        {43, 55, 20},     // Building 4
        {53, 62, 12},     // Small building
        {60, 75, 28},     // Tall building (signal building)
        {73, 82, 8},      // Small
        {80, 95, 22},     // Building 6
        {93, 108, 18},    // Building 7
        {106, 120, 30},   // Another tall one
        {118, 132, 14},   // Building 8
        {130, 144, 10}    // Right edge
    };

    int num_buildings = sizeof(buildings) / sizeof(buildings[0]);

    // Scale buildings for round display
    #ifdef PBL_ROUND
    float scale_x = (float)s_screen_width / 144.0f;
    #endif

    // Draw building silhouettes
    graphics_context_set_fill_color(ctx, COLOR_SKYLINE);

    for (int i = 0; i < num_buildings; i++) {
        #ifdef PBL_ROUND
        int16_t x1 = (int16_t)(buildings[i].x1 * scale_x);
        int16_t x2 = (int16_t)(buildings[i].x2 * scale_x);
        #else
        int16_t x1 = buildings[i].x1;
        int16_t x2 = buildings[i].x2;
        #endif

        GRect building = GRect(x1, sy - buildings[i].h,
                               x2 - x1, buildings[i].h + 50);
        graphics_fill_rect(ctx, building, 0, GCornerNone);
    }

    // Ground fill
    GRect ground = GRect(0, sy, s_screen_width, s_screen_height - sy);
    graphics_context_set_fill_color(ctx, COLOR_SKYLINE_DARK);
    graphics_fill_rect(ctx, ground, 0, GCornerNone);

    // Window lights (scattered yellow dots)
    #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, COLOR_WINDOWS);
    #else
    graphics_context_set_fill_color(ctx, COLOR_WINDOWS);
    #endif

    // Pre-defined window positions
    GPoint windows[] = {
        {15, sy - 18}, {17, sy - 15}, {15, sy - 10},
        {35, sy - 28}, {38, sy - 25}, {36, sy - 20}, {40, sy - 15},
        {65, sy - 22}, {68, sy - 18}, {70, sy - 24},
        {86, sy - 16}, {89, sy - 12},
        {110, sy - 24}, {113, sy - 20}, {115, sy - 26}, {112, sy - 15},
        {125, sy - 10}, {128, sy - 8}
    };

    int num_windows = sizeof(windows) / sizeof(windows[0]);

    for (int i = 0; i < num_windows; i++) {
        #ifdef PBL_ROUND
        int16_t wx = (int16_t)(windows[i].x * scale_x);
        #else
        int16_t wx = windows[i].x;
        #endif

        if (wx < s_screen_width - 2) {
            graphics_fill_rect(ctx, GRect(wx, windows[i].y, 2, 2), 0, GCornerNone);
        }
    }

    // Searchlight housing on rooftop (centered building)
    int housing_x = s_center_x - 4;
    int housing_y = sy - 28;
    graphics_context_set_fill_color(ctx, COLOR_SKYLINE_DARK);
    graphics_fill_rect(ctx, GRect(housing_x, housing_y, 8, 6), 0, GCornerNone);

    #ifdef PBL_COLOR
    // Searchlight glow at base
    graphics_context_set_fill_color(ctx, COLOR_BEAM_BRIGHT);
    graphics_fill_circle(ctx, GPoint(s_center_x, housing_y + 3), 3);
    #endif
}

static void draw_battery(GContext *ctx) {
    // Battery indicator in top-right
    int bx = s_screen_width - 26;
    int by = 6;

    // Battery outline
    graphics_context_set_stroke_color(ctx, COLOR_TIME);
    graphics_draw_rect(ctx, GRect(bx, by, 20, 10));
    graphics_fill_rect(ctx, GRect(bx + 20, by + 3, 2, 4), 0, GCornerNone);

    // Battery fill
    int fill_width = (16 * s_battery_level) / 100;

    GColor fill_color;
    if (s_battery_level > 50) {
        fill_color = COLOR_BATTERY_GOOD;
    } else if (s_battery_level > 20) {
        fill_color = COLOR_BATTERY_MED;
    } else {
        fill_color = COLOR_BATTERY_LOW;
    }

    graphics_context_set_fill_color(ctx, fill_color);
    graphics_fill_rect(ctx, GRect(bx + 2, by + 2, fill_width, 6), 0, GCornerNone);

    // Charging indicator
    if (s_is_charging) {
        graphics_context_set_stroke_color(ctx, COLOR_TIME);
        // Lightning bolt
        graphics_draw_line(ctx, GPoint(bx + 10, by + 1), GPoint(bx + 7, by + 5));
        graphics_draw_line(ctx, GPoint(bx + 7, by + 5), GPoint(bx + 12, by + 5));
        graphics_draw_line(ctx, GPoint(bx + 12, by + 5), GPoint(bx + 9, by + 9));
    }
}

// ============================================================================
// CANVAS UPDATE
// ============================================================================

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    // 1. Draw night sky
    draw_sky(ctx, bounds);

    // 2. Draw stars
    draw_stars(ctx);

    // 3. Draw searchlight beam (behind bat and skyline)
    draw_searchlight_beam(ctx);

    // 4. Draw bat symbol
    draw_bat_symbol(ctx);

    // 5. Draw Gotham skyline (in front of beam)
    draw_skyline(ctx);

    // 6. Draw battery indicator
    draw_battery(ctx);
}

// ============================================================================
// ANIMATION TIMER
// ============================================================================

static void animation_timer_callback(void *data) {
    // Update all animated elements
    update_searchlight();
    update_bat_symbol();
    update_stars();

    // Redraw
    if (s_canvas_layer) {
        layer_mark_dirty(s_canvas_layer);
    }

    // Schedule next frame
    s_animation_timer = app_timer_register(s_animation_interval, animation_timer_callback, NULL);
}

// ============================================================================
// EVENT HANDLERS
// ============================================================================

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    // Update time
    if (clock_is_24h_style()) {
        strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
    } else {
        strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
        // Remove leading zero
        if (s_time_buffer[0] == '0') {
            memmove(s_time_buffer, s_time_buffer + 1, strlen(s_time_buffer));
        }
    }
    text_layer_set_text(s_time_layer, s_time_buffer);

    // Update date
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %b %d", tick_time);
    text_layer_set_text(s_date_layer, s_date_buffer);
}

static void battery_handler(BatteryChargeState charge) {
    s_battery_level = charge.charge_percent;
    s_is_charging = charge.is_charging;

    // Adjust animation speed based on battery
    if (s_battery_level <= LOW_BATTERY_THRESHOLD && !s_is_charging) {
        s_animation_interval = ANIMATION_INTERVAL_LOW_POWER;
    } else {
        s_animation_interval = ANIMATION_INTERVAL;
    }
}

// ============================================================================
// WINDOW HANDLERS
// ============================================================================

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Set screen dimensions
    s_screen_width = bounds.size.w;
    s_screen_height = bounds.size.h;
    s_center_x = s_screen_width / 2;

    #ifdef PBL_ROUND
    s_skyline_top = 140;
    #else
    s_skyline_top = 130;
    #endif

    // Create canvas layer
    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

    // Create bat path
    s_bat_path = gpath_create(&s_bat_path_info);

    // Time layer
    #ifdef PBL_ROUND
    GRect time_rect = GRect(0, 18, bounds.size.w, 50);
    #else
    GRect time_rect = GRect(0, 8, bounds.size.w, 50);
    #endif
    s_time_layer = text_layer_create(time_rect);
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, COLOR_TIME);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // Date layer
    #ifdef PBL_ROUND
    GRect date_rect = GRect(0, 60, bounds.size.w, 24);
    #else
    GRect date_rect = GRect(0, 52, bounds.size.w, 24);
    #endif
    s_date_layer = text_layer_create(date_rect);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, COLOR_DATE);
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Initialize components
    init_searchlight();
    init_bat_symbol();
    init_stars();

    // Get initial battery state
    battery_handler(battery_state_service_peek());

    // Update time immediately
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    tick_handler(tick_time, MINUTE_UNIT);

    // Start animation timer
    s_animation_timer = app_timer_register(s_animation_interval, animation_timer_callback, NULL);
}

static void window_unload(Window *window) {
    // Cancel timer
    if (s_animation_timer) {
        app_timer_cancel(s_animation_timer);
        s_animation_timer = NULL;
    }

    // Destroy paths
    if (s_bat_path) {
        gpath_destroy(s_bat_path);
        s_bat_path = NULL;
    }

    // Destroy layers
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    layer_destroy(s_canvas_layer);
}

// ============================================================================
// MAIN
// ============================================================================

static void init(void) {
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    window_set_background_color(s_main_window, GColorBlack);
    window_stack_push(s_main_window, true);

    // Subscribe to services
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_handler);
}

static void deinit(void) {
    battery_state_service_unsubscribe();
    tick_timer_service_unsubscribe();
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
