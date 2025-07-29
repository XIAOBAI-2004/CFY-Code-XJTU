`timescale 1ns / 1ps
module Countdown_LED(
    input clk,
    input rst_n,
    input led,
    input state,
    output reg [7:0] light
    );

    always@(posedge clk,negedge rst_n)
    if(!rst_n) begin
          light[0]<=4'd0;light[1]<=4'd0;light[2]<=4'd0;light[3]<=4'd0;
          light[4]<=4'd0;light[5]<=4'd0;light[6]<=4'd0;light[7]<=4'd0;
    end
    else if (state==0&&led==0) begin     //正计时
          light[0]<=4'd1;light[1]<=4'd1;light[2]<=4'd1;light[3]<=4'd1;
          light[4]<=4'd0;light[5]<=4'd0;light[6]<=4'd0;light[7]<=4'd0;
    end
    else if (state==1&&led==0) begin     //倒计时
          light[0]<=4'd0;light[1]<=4'd0;light[2]<=4'd0;light[3]<=4'd0;
          light[4]<=4'd1;light[5]<=4'd1;light[6]<=4'd1;light[7]<=4'd1;
    end
    else if (led==1) begin  //计时结束
          light[0]<=4'd1;light[1]<=4'd1;light[2]<=4'd1;light[3]<=4'd1;
          light[4]<=4'd1;light[5]<=4'd1;light[6]<=4'd1;light[7]<=4'd1;
    end
    else begin
          light[0]<=4'd0;light[1]<=4'd0;light[2]<=4'd0;light[3]<=4'd0;
          light[4]<=4'd0;light[5]<=4'd0;light[6]<=4'd0;light[7]<=4'd0;
    end
    
endmodule
