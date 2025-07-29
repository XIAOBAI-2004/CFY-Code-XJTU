`timescale 1ns / 1ps
module Countdown_time_setup(
    input clk,
    input rst_n,
    input load_en,
    input second_low,
    input second_high,
    input minute_low,
    input minute_high,
    input key_plus,
    output reg [15:0] t //ʱ��
    );

    reg key_plus_last;//���浱ǰ����״̬����һ��ʱ�����ڵļĴ���
    reg rising_edge;//������

always @(posedge clk) begin
    key_plus_last<=key_plus; 
    rising_edge<=~key_plus_last&key_plus; // ���������
end
    
always@(posedge clk or negedge rst_n) begin
if (!rst_n) t[15:0]<=16'd0;
else if (rising_edge) begin
    if (second_low==1&&t[3:0]==9) t[3:0]<=4'd0;
    else if (second_low==1&&t[3:0]!=9) t[3:0]<=t[3:0]+4'd1;

    else if (second_high==1&&t[7:4]==5) t[7:4]<=4'd0;
    else if (second_high==1&&t[7:4]!=5) t[7:4]<=t[7:4]+4'd1;

    else if (minute_low==1&&t[11:8]==9) t[11:8]<=4'd0;
    else if (minute_low==1&&t[11:8]!=9) t[11:8]<=t[11:8]+4'd1;

    else if (minute_high==1&&t[15:12]==9) t[15:12]<=4'd0;
    else if (minute_high==1&&t[15:12]!=9) t[15:12]<=t[15:12]+4'd1;
end
    else t<=t;
end

endmodule
