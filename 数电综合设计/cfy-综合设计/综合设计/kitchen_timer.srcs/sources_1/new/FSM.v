`timescale 1ns / 1ps
module FSM(
    input clk,         
    input rst_n, 
    input state_choose,  //ģʽѡ��          
    input key_start_stop,  //��ͣ�ָ���ʱ�ź�
    input key_load,          //�����ź�
    output reg state,     //ģʽ�����0=����ʱ,1=����ʱ
    output reg cnt_en,   //����ʹ���ź�
    output reg load_en //��������
    );
    
    //״̬����
    reg [2:0] cstate; //����̬
    reg [2:0] nstate=0; //���̬
    parameter 
    //����ʱ
    WORK_INIT0=3'b000,COUNT0=3'b001,PAUSE0=3'b010,
    //����ʱ
    WORK_INIT1=3'b100,COUNT1=3'b101,PAUSE1=3'b110;  
    
    //��һ�Σ�״̬ת�ƣ�cstate ����
    always @(posedge clk or negedge rst_n) 
    if (!rst_n)
        cstate<=WORK_INIT0;  
    else 
        cstate<=nstate;
        
     //�ڶ��Σ���̬�жϣ�nstate
     always @ *
     begin
     case (cstate)
     //����ʱ state=0
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
     //����ʱ state=1
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
     
//�����Σ���̬���
//cnt_en:���޼������ڼ����Ļ�����1
always @ (posedge clk or negedge rst_n)
if (!rst_n)  
    cnt_en<=0;
else if(cstate==COUNT0)
    cnt_en<=1;
else if(cstate==COUNT1)
    cnt_en<=1;
else cnt_en<=0;

//load_en:�����������еĻ�����1
always @ (posedge clk or negedge rst_n)
if (!rst_n) 
    load_en<=1;
else if(cstate==COUNT1||cstate==PAUSE1)//����ʱ
    load_en<=0;
else if(cstate==COUNT0||cstate==PAUSE0)//����ʱ
    load_en<=0;
else load_en<=1;  

//state��ģʽ������ʱ=0 ����ʱ=1
always @ (posedge clk or negedge rst_n)
if (!rst_n) 
    state<=0;  //��λ��������Ϊ����ʱ
else if (cstate==COUNT1) state<=1;
else if (cstate==PAUSE1) state<=1;
else if (cstate==WORK_INIT1) state<=1;
else if (cstate==COUNT0) state<=0;
else if (cstate==PAUSE0) state<=0;
else if (cstate==WORK_INIT0) state<=0;
else state<=0;

endmodule