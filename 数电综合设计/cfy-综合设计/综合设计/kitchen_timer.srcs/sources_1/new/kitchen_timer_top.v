`timescale 1ns / 1ps
module kitchen_timer_top(
    input clk,rst_n,
    input state_choose, //正计时倒计时 模式选择按键
    input key_start_stop, //开始暂停
    input key_load,  //置数
    input second_low,second_high,minute_low,minute_high, //控制调节按钮
    input key_plus, //调节按钮（加数）
    output wire [6:0] a_to_g,
    output wire [3:0] an,
    output wire [7:0] light
    );
    
    //中间变量
    wire state,cnt_en,load_en; //FSM
    wire sec_pulse; //sec_pulse
    wire [15:0]t; //Countdown_time_setup
    wire [15:0]x,led; //cnt
    wire key_p_flag_key_start_stop,key_p_flag_key_load,key_p_flag_state_choose,key_p_flag_key_plus; //消抖的
    
    debounce debounce_key_start_stop(
    .clk(clk),
    .rst_n(rst_n),
    .key(key_start_stop),
    .key_p_flag(key_p_flag_key_start_stop)
    );
    
    debounce debounce_key_load(
    .clk(clk),
    .rst_n(rst_n),
    .key(key_load),
    .key_p_flag(key_p_flag_key_load)
    );
    
    debounce debounce_state_choose(
    .clk(clk),
    .rst_n(rst_n),
    .key(state_choose),
    .key_p_flag(key_p_flag_state_choose)
    );
    
    debounce debounce_key_plus(
    .clk(clk),
    .rst_n(rst_n),
    .key(key_plus),
    .key_p_flag(key_p_flag_key_plus)
    );
       
    FSM uut1_FSM(
    .clk(clk),
    .rst_n(rst_n),
    .key_start_stop(key_p_flag_key_start_stop),
    .key_load(key_p_flag_key_load),
    .state_choose(key_p_flag_state_choose),
    .state(state),
    .cnt_en(cnt_en),
    .load_en(load_en)
    );
    
    sec_pulse uut2_sec_pulse(
    .clk(clk),
    .rst_n(rst_n),
    .cnt_en(cnt_en),
    .sec_pulse(sec_pulse)
    );
    
    Countdown_time_setup uut3_time_set(
    .clk(clk),
    .rst_n(rst_n),
    .load_en(load_en),
    .second_low(second_low),
    .second_high(second_high),
    .minute_low(minute_low),
    .minute_high(minute_high),
    .key_plus(key_p_flag_key_plus),
    .t(t)
    );
    
    reversible_cnt uut4_cnt(
    .clk(clk),
    .rst_n(rst_n),
    .cnt_en(cnt_en),
    .sec_pulse(sec_pulse),
    .state(state),
    .load_en(load_en),
    .t(t),
    .x(x),
    .led(led)
    );
    
    display uut5_display(
    .clk(clk),
    .rst_n(rst_n),
    .x(x), 
    .a_to_g(a_to_g), 
    .an(an) 
    );
    
    Countdown_LED uut6_Countdown_LED(
    .clk(clk),
    .rst_n(rst_n),
    .state(state),
    .led(led),
    .light(light)
    );
    
endmodule
