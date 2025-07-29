`timescale 1ns / 1ps
module reversible_cnt(
    input clk,
    input rst_n,
    input sec_pulse,
    input state,
    input cnt_en,
    input load_en,
    input [15:0] t,  //倒计时初值
    output reg [15:0] x, 
    output reg led   //提醒LED
    );
    
    //正计时：state=0
    always@(posedge clk or negedge rst_n)
        if (!rst_n) begin 
            x<=16'd0;
            led<=0;
        end
        
        //正计时模式
        else if (sec_pulse==1&&cnt_en==1&&state==0) begin 
                if (x[3:0]!==9) begin
                    x[3:0]<=x[3:0]+4'd1;
                    x[7:4]<=x[7:4];
                    x[11:8]<=x[11:8];
                    x[15:12]<=x[15:12];
                    led<=0;
                end
                else if (x[3:0]==9&&x[7:4]!==5) begin
                    x[3:0]<=4'd0;
                    x[7:4]<=x[7:4]+4'd1;
                    x[11:8]<=x[11:8];
                    x[15:12]<=x[15:12];
                    led<=0;
                end
                else if (x[3:0]==9&&x[7:4]==5&&x[11:8]!=9) begin
                    x[3:0]<=4'd0;
                    x[7:4]<=4'd0;
                    x[11:8]<=x[11:8]+4'd1;
                    x[15:12]<=x[15:12];
                    led<=0;
                end
                else if (x[3:0]==9&&x[7:4]==5&&x[11:8]==9&&x[15:12]!=9) begin
                    x[3:0]<=4'd0;
                    x[7:4]<=4'd0;
                    x[11:8]<=4'd0;
                    x[15:12]<=x[15:12]+4'd1;
                    led<=0;
                end
                else if (x[3:0]==9&&x[7:4]==5&&x[11:8]==9&&x[15:12]==9) begin //计时结束
                    x[15:0]<=x[15:0];
                    led<=1;
                end
          end
          else if (load_en==1&&state==0)begin
                x<=16'd0;
                led<=0;
          end

        //倒计时：state=1
        else if (state==1&&load_en==1) begin  //倒计时模式
            x[15:0]<=t[15:0];
            led<=0;
        end
        else if (state==1&&cnt_en==1&&sec_pulse==1) begin  //开始倒计时
                if (x[3:0]!==0) begin
                    x[3:0]<=x[3:0]-4'd1;
                    x[7:4]<=x[7:4];
                    x[11:8]<=x[11:8];
                    x[15:12]<=x[15:12];
                    led<=0;
                end
                else if (x[3:0]==0&&x[7:4]!==0) begin
                    x[3:0]<=4'd9;
                    x[7:4]<=x[7:4]-4'd1;
                    x[11:8]<=x[11:8];
                    x[15:12]<=x[15:12];
                    led<=0;
                end
                else if (x[3:0]==0&&x[7:4]==0&&x[11:8]!==0) begin
                    x[3:0]<=4'd9;
                    x[7:4]<=4'd5;
                    x[11:8]<=x[11:8]-4'd1;
                    x[15:12]<=x[15:12];
                    led<=0;
                end
                else if (x[3:0]==0&&x[7:4]==0&&x[11:8]==0&&x[15:12]!=0) begin
                    x[3:0]<=4'd9;
                    x[7:4]<=4'd5;
                    x[11:8]<=4'd9;
                    x[15:12]<=x[15:12]-4'd1;
                    led<=0;
                end
                else if (x[3:0]==0&&x[7:4]==0&&x[11:8]==0&&x[15:12]==0) begin //倒计时结束 
                    x[15:0]<=x[15:0];
                    led<=1;
                end
            end
endmodule
