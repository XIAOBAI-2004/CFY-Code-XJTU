`timescale 1ns / 1ps
module display(
    input clk,
    input rst_n,
    input [15:0]x,
    output reg [6:0]a_to_g, //控制7个管，段信号
    output reg [3:0]an  //位选，分钟、秒数各两个
    );
    
    //clkdiv 时钟分频
    reg [20:0] clkdiv;  
    always @(posedge clk, negedge rst_n)
    begin
        if(!rst_n) clkdiv<=21'd0;
        else clkdiv<=clkdiv+1;
    end 
    
    //bitcnt 位扫描
    wire [1:0] bitcnt;
    assign bitcnt=clkdiv[20:19];
    
    //an位选信号的产生，高有效
    always@*
    begin
        if(!rst_n)
        an=4'b0;
    else begin
        an=4'b0;
        an[bitcnt]=1;
    end
    end
    
    //digit 保存当前显示的数字
    reg [3:0] digit;
    always @* begin
        if(!rst_n)
            digit=4'd0;
        else
            case(bitcnt)
                2'd0:digit=x[3:0];
                2'd1:digit=x[7:4];
                2'd2:digit=x[11:8];
                2'd3:digit=x[15:12];
            endcase
        end
        
        //译码
        always@(*) begin
        if(!rst_n)
        a_to_g=7'b1111111;
        else case(digit)
            0:a_to_g=7'b1111110;
            1:a_to_g=7'b0110000;
            2:a_to_g=7'b1101101;
            3:a_to_g=7'b1111001;
            4:a_to_g=7'b0110011;
            5:a_to_g=7'b1011011;
            6:a_to_g=7'b1011111;
            7:a_to_g=7'b1110000;
            8:a_to_g=7'b1111111;
            9:a_to_g=7'b1111011;
            'hA:a_to_g=7'b1110111;
            'hB:a_to_g=7'b0011111;
            'hC:a_to_g=7'b1001110;
            'hD:a_to_g=7'b0111101;
            'hE:a_to_g=7'b1001111;
            'hF:a_to_g=7'b1000111;
            default:a_to_g=7'b1111110;  //0
        endcase
end
endmodule
    
    
