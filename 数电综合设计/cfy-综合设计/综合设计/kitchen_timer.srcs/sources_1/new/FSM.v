`timescale 1ns / 1ps
module FSM(
    input clk,         
    input rst_n, 
    input state_choose,  //模式选择          
    input key_start_stop,  //暂停恢复计时信号
    input key_load,          //置数信号
    output reg state,     //模式输出：0=正计时,1=倒计时
    output reg cnt_en,   //计数使能信号
    output reg load_en //有无置数
    );
    
    //状态编码
    reg [2:0] cstate; //存现态
    reg [2:0] nstate=0; //存次态
    parameter 
    //正计时
    WORK_INIT0=3'b000,COUNT0=3'b001,PAUSE0=3'b010,
    //倒计时
    WORK_INIT1=3'b100,COUNT1=3'b101,PAUSE1=3'b110;  
    
    //第一段，状态转移：cstate 更新
    always @(posedge clk or negedge rst_n) 
    if (!rst_n)
        cstate<=WORK_INIT0;  
    else 
        cstate<=nstate;
        
     //第二段，次态判断：nstate
     always @ *
     begin
     case (cstate)
     //正计时 state=0
     WORK_INIT0: 
        if (key_start_stop==1)  nstate=COUNT0;   
        else if (state_choose==1) nstate=WORK_INIT1;
        else nstate = cstate;
     COUNT0:
        if (key_start_stop==1) nstate=PAUSE0;
        else nstate = cstate;
     PAUSE0:
        if (key_start_stop==1) nstate=COUNT0;
        else if (key_load==1) nstate=WORK_INIT0;
        else nstate=cstate;
     //倒计时 state=1
     WORK_INIT1:
        if (key_start_stop==1) nstate=COUNT1;
        else if (state_choose==1) nstate=WORK_INIT0;
        else nstate=cstate;
     COUNT1:
        if (key_start_stop==1) nstate=PAUSE1;
        else if (key_load==1) nstate=WORK_INIT1;
        else nstate=cstate;
    PAUSE1:
        if (key_start_stop==1) nstate=COUNT1;
        else if (key_load==1) nstate=WORK_INIT1;
        else nstate=cstate;
    default:nstate=WORK_INIT0;
    endcase
    end
     
//第三段，次态输出
//cnt_en:有无计数，在计数的话就是1
always @ (posedge clk or negedge rst_n)
if (!rst_n)  
    cnt_en<=0;
else if(cstate==COUNT0)
    cnt_en<=1;
else if(cstate==COUNT1)
    cnt_en<=1;
else cnt_en<=0;

//load_en:有无置数，有的话就是1
always @ (posedge clk or negedge rst_n)
if (!rst_n) 
    load_en<=1;
else if(cstate==COUNT1||cstate==PAUSE1)//倒计时
    load_en<=0;
else if(cstate==COUNT0||cstate==PAUSE0)//正计时
    load_en<=0;
else load_en<=1;  

//state：模式，正计时=0 倒计时=1
always @ (posedge clk or negedge rst_n)
if (!rst_n) 
    state<=0;  //复位是先设置为正计时
else if (cstate==COUNT1) state<=1;
else if (cstate==PAUSE1) state<=1;
else if (cstate==WORK_INIT1) state<=1;
else if (cstate==COUNT0) state<=0;
else if (cstate==PAUSE0) state<=0;
else if (cstate==WORK_INIT0) state<=0;
else state<=0;

endmodule