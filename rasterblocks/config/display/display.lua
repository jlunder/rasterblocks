pal_black_red_gold_white_fs = rb.palette_from_pwl({
    {rb.color(0.00, 0.00, 0.00, 1), 2},
    {rb.color(0.25, 0.00, 0.00, 1), 2},
    {rb.color(0.50, 0.12, 0.00, 1), 2},
    {rb.color(1.00, 0.25, 0.00, 1), 1},
    {rb.color(1.00, 0.50, 0.00, 1), 1},
    {rb.color(1.00, 1.00, 0.00, 1), 1},
    {rb.color(1.00, 1.00, 1.00, 1), 0},
})

--pal_black_red_gold_white_fs_alpha = rb.alpha_palette_from_palette(pal_black_red_gold_white_fs)

pal_black_blue_green_white_fs = rb.palette_from_pwl({
    {rb.color(0.00, 0.00, 0.00, 1), 1},
    {rb.color(0.00, 0.25, 1.00, 1), 1},
    {rb.color(0.50, 0.00, 0.25, 1), 1},
    {rb.color(0.00, 1.00, 0.25, 1), 1},
    {rb.color(1.00, 1.00, 1.00, 1), 0},
})

rb.set_generator(
    rb.gen_controller_fade(
        rb.gen_compositor(
--            rb.gen_pulse_checkerboard(rb.color(1, .5, .5)),        
--            rb.gen_oscilloscope(rb.color(1, 1, 1))        
            rb.gen_fireworks(pal_black_blue_green_white_fs, 0)        
        ),
        7
    )
)

--pal_black_blue_green_white_fs_alpha = rb.alpha_palette_from_palette(pal_black_blue_green_white_fs)

--[[
static RBPiecewiseLinearColorSegment g_rbPalBlackWhiteFS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{1.00, 1.00, 1.00, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalRainbowFS[] = {
    {{1.00, 0.00, 0.00, 1}, 1},
    {{0.00, 1.00, 0.00, 1}, 1},
    {{0.00, 0.00, 1.00, 1}, 1},
    {{1.00, 0.00, 0.00, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackGoldFS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{1.00, 1.00, 0.25, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPaleBlueFS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{0.25, 0.25, 1.00, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPaleGreenFS[] = {
    {{0.00, 0.00, 0.00, 0}, 1},
    {{0.25, 1.00, 0.25, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPurpleFS[] = {
    {{0.00, 0.00, 0.00, 0}, 1},
    {{0.50, 0.00, 1.00, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalGreenLavenderHS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{0.03, 0.12, 0.03, 1}, 1},
    {{0.36, 0.12, 0.50, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPurpleRedHS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{0.12, 0.00, 0.25, 1}, 1},
    {{0.50, 0.06, 0.06, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPurpleHS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{0.06, 0.00, 0.12, 1}, 1},
    {{0.36, 0.00, 0.50, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBluePurpleGreenHS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{0.00, 0.12, 0.50, 1}, 1},
    {{0.25, 0.00, 0.12, 1}, 1},
    {{0.00, 0.50, 0.12, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlueGoldHS[] = {
    {{0.00, 0.06, 0.12, 1}, 1},
    {{0.00, 0.25, 0.50, 1}, 1},
    {{0.50, 0.50, 0.06, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalRedPinkHS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{0.36, 0.00, 0.00, 1}, 1},
    {{0.50, 0.25, 0.25, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackRedGoldHS[] = {
    {{0.00, 0.00, 0.00, 1}, 2},
    {{0.12, 0.00, 0.00, 1}, 2},
    {{0.25, 0.03, 0.00, 1}, 2},
    {{0.50, 0.12, 0.00, 1}, 1},
    {{0.50, 0.25, 0.00, 1}, 1},
    {{0.50, 0.50, 0.06, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackWhiteHS[] = {
    {{0.00, 0.00, 0.00, 1}, 1},
    {{0.50, 0.50, 0.50, 1}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalImageOverlay[] = {
    {{0.50, 0.50, 0.50, 1}, 1},
    {{1.00, 1.00, 1.00, 1}, 0},
};
]]

