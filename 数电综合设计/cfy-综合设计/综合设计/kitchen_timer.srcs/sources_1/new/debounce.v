`timescale 1ns / 1ps
module debounce(
    input clk,
    input rst_n,
    input key,
    output reg key_p_flag
    );
    
parameter DELAY_TIME=21'd1500000; //延时15ms

//key_reg:按键输入寄存器
reg key_reg;
always @(posedge clk or negedge rst_n)
    if(!rst_n)
        key_reg<=0;
    else
        key_reg<=key;
        
//delay_cnt延时计数器
reg [20:0] delay_cnt;
always @(posedge clk or negedge rst_n)
    if(!rst_n)
        delay_cnt<=21'b0;
    else if(key!=key_reg)
        delay_cnt<=DELAY_TIME;
    else if(delay_cnt>0)
        delay_cnt<=delay_cnt-1'b1;
    else
        delay_cnt<=21'b0;  

//有效变化
reg key_value;
always @(posedge clk or negedge rst_n)
    if(!rst_n)
        key_value<=1'b0;
    else if(delay_cnt==1'b1)
        key_value<=key;
    else
        key_value<=key_value;

//key_p_flag:按键上升标志信号
always@(posedge clk or negedge rst_n)
    if(!rst_n)
        key_p_flag<=1'b0;
    else if(delay_cnt==1'b1&&key==1)
        key_p_flag<=1'b1;
    else
        key_p_flag<=1'b0;
        
endmodule
