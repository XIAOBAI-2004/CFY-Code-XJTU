`timescale 1ns / 1ps
module sec_pulse(
    input clk,
    input rst_n,
    input cnt_en,  //��ͣ��ʼʹ��
    output reg sec_pulse
    );
    
    parameter fequency=27'd100_000_000; //100Mhz��Ƶ
    parameter max_counter=fequency-1;
    reg [30:0] counter; //�㹻��
    
    //counter ʱ��
    always@(posedge clk or negedge rst_n)
    if (!rst_n)
        counter<=0;
    else if (counter==max_counter)
        counter<=0;
    else 
        counter<=counter+1;
        

    //sec_pulse
     always@(posedge clk or negedge rst_n)
     if (!rst_n)
        sec_pulse<=0;
     else if (counter==max_counter)
        sec_pulse<=1;
     else
        sec_pulse<=0;
        
endmodule

