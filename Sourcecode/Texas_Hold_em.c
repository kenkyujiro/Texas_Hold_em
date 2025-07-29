#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

//Cygwinとwindowsでコンパイルを変える
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

struct Playing{//
    int cip;//合計チップ数
    int card[5];//出目
    int total_raise;//レイズした回数
    bool falled;//フォールドしたかどうか
    bool called;//コールしたかどうか
};

void getcard(int all_card[], int card[], int size);
void sort(int subject_card[], int size);
int Toparent(int p_card[], int c1_card[], int c2_card[], int c3_card[]);
int Judge(int c_card[], int show_number, int judge1, int judge2, int judge3);
int thought3(int card[]);
int thought4(int card[]);
int thought5(int card[]);
void call(struct Playing *player, int *bet_cip, int *pool);
void raiseBet(struct Playing *player, bool *called1, bool *called2, bool *called3, int *bet_cip, int *pool);
void fold(bool *fold, bool *call);
int showdown(int score[5]);
int Who_round_winner(int Get_point[4], bool Be_fold[4]);
int Who_game_winner(int point1, int point2, int point3, int point4);
void my_sleep(unsigned int seconds);

int main(){
    int dealer[3];//ディーラーの手札(山札?)
    int all[52] = {1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,8,9,9,9,9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13};
    int hold_card[52];//開始毎にリセットする用
    int *challengep,*pool;//掛けるチップ用の変数
    int challengepoint=0,poolpoint=0;//ポインタにアドレスをセットする用
    int get_point[4]={0,0,0,0};//獲得ポイント(出目)
    bool be_fold[4];//foldを管理しやすくする用
    int i,j;//繰り返し用変数
    int shuffle_point,shuffle_card;//シャッフル用変数
    int right_command;//コール、レイズ、フォールドのいずれかやったか判断する
    int all_call,how_round;//全員コール出来る用変数
    int command_player;//数字入力用変数
    int command_cpu1,command_cpu2,command_cpu3;//cpu判断用
    int parent;//親回し用変数。最初にJが配られた人が親
    int who_winner;//誰が勝ったか判断させる変数
    srand(time(NULL));

    struct Playing player;
    struct Playing cpu1;
    struct Playing cpu2;
    struct Playing cpu3;
    player.cip = 1000;
    cpu1.cip = 1000;
    cpu2.cip = 1000;
    cpu3.cip = 1000;

    for(i=0;i<52;i++){//カードをセット
        hold_card[i] = all[i];
    }

    // ポインタにアドレスをセット
    challengep = &challengepoint;
    pool = &poolpoint;

    //ゲーム開始!!
    for(i=0;i<5;i++)//一ラウンドの流れをまとめたやつ※whlie文で繰り返している。
    {
        *challengep = 0;
        *pool = 0;
        for(j=0;j<52;j++){//カードリセット
            all[j] = hold_card[j];
        }
        printf("ラウンド%d\n",i+1);

        for(j=0;j<52;j++){//カードシャッフル
            shuffle_point = rand()%52;
            shuffle_card = all[j];
            all[j] = all[shuffle_point];
            all[shuffle_point] = shuffle_card;
        }
        

        getcard(all, player.card, 2);
        getcard(all, cpu1.card, 2);
        getcard(all, cpu2.card, 2);
        getcard(all, cpu3.card, 2);
        getcard(all, dealer, 3);

        for(j=0;j<3;j++)//ディーラーの手札(全体に配ってのちに表示していく)
        {
            player.card[j+2] = dealer[j];
            cpu1.card[j+2] = dealer[j];
            cpu2.card[j+2] = dealer[j];
            cpu3.card[j+2] = dealer[j];
        }

        //親決め、戻り値は1～4。
        parent = Toparent(player.card, cpu1.card, cpu2.card, cpu3.card);        
        
        printf("あなたの手札は\n");
        for(j=0;j<2;j++)
        {
            printf("%d枚目：%d　",j+1,player.card[j]);
        }

        //リセット処理
        player.called = false;
        player.falled = false;
        cpu1.called = false;
        cpu1.falled = false;
        cpu2.called = false;
        cpu2.falled = false;
        cpu3.called = false;
        cpu3.falled = false;
        command_player = 0;
        command_cpu1 = 0;
        command_cpu2 = 0;
        command_cpu3 = 0;
        my_sleep(4);


        //掛金ベットフェーズ
        //レイズやフォールド、そして徐々にディーラーの手札開示
        for(how_round=0;how_round<3;how_round++){
            printf("\nフェーズ%d\n",how_round+1);
            printf("新たに開示されるカードは、%d\n", dealer[how_round]);
            my_sleep(4);
            printf("\n");


            all_call=0;
            while(all_call==0){//全員コールすることで繰り返しを抜ける cpuが最初の場合は20懸け、レイズは二倍懸ける                

                //親によってコールの順番を変える。
                if(parent == 1){
                    right_command = 0;//バグ防止リセット
                    if(player.falled == false){
                        while(right_command==0){//正しいコマンドを入力するまで終わらない用
                        
                        printf("現在の掛けチップ数：%d、現在のチッププール数：%d\n",*challengep,*pool);
                        printf("フォールド=１、コール=２、レイズ=３、カードの確認=４\n");//コールか、レイズか、フォールドか(全員コールで抜けられる)
                        printf("あなたの番です。どうしますか？：");
                        scanf("%d",&command_player);

                        if(command_player == 2 && *challengep == 0){//初めてのコール
                            printf("いくつチップを賭けますか？：");
                            scanf("%d",challengep);
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 3 && *challengep == 0){//レイズ回避用
                            printf("いくつチップを賭けますか？：");
                            scanf("%d",challengep);
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 1){//フォールド
                            fold(&player.falled, &player.called);
                            right_command = 1;
                        }
                        else if(command_player == 2){//コール
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 3){//レイズ
                            raiseBet(&player, &cpu1.called, &cpu2.called, &cpu3.called, challengep, pool);
                            player.total_raise += 1;
                            right_command = 1;
                        }
                        else if(command_player == 4){
                            printf("あなたのカード\n");
                            for(j=0;j<2;j++){
                                printf("%d ", player.card[j]);
                            }
                            printf("\n場のカード\n");
                            for(j=2;j<=how_round+2;j++){
                                printf("%d ", player.card[j]);
                            }
                            printf("\n");
                        }
                        else{
                            printf("エラー。違う数を入力してください。\n");
                        }
                        }
                    }

                    //cpuの判断を出力。kはディーラーの公開札数
                    if(cpu1.falled == false){
                        printf("CPU1のターン\n");
                        command_cpu1 = Judge(cpu1.card, how_round, command_player, command_cpu2, command_cpu3);//この下にレイズなどの処理を出力結果を用いて行う.kとは？
                        if(command_cpu1 == 1){//フォールド
                            fold(&cpu1.falled, &cpu1.called);
                        }
                        else if(command_cpu1 == 2){//コール
                            call(&cpu1, challengep, pool);
                        }
                        else if(command_cpu1 == 3){//レイズ
                            raiseBet(&cpu1, &player.called, &cpu2.called, &cpu3.called, challengep, pool);
                            cpu1.total_raise += 1;
                        }
                    }
                    if(cpu2.falled == false){
                        printf("CPU2のターン\n");
                        command_cpu2 = Judge(cpu2.card, how_round, command_player, command_cpu1, command_cpu3);
                        if(command_cpu2 == 1){//フォールド
                            fold(&cpu2.falled, &cpu2.called);
                        }
                        else if(command_cpu2 == 2){//コール
                            call(&cpu2, challengep, pool);
                        }
                        else if(command_cpu2 == 3){//レイズ
                            raiseBet(&cpu2, &player.called, &cpu1.called, &cpu3.called, challengep, pool);
                            cpu2.total_raise += 1;
                        }
                    }
                    if(cpu3.falled == false){
                        printf("CPU3のターン\n");
                        command_cpu3 = Judge(cpu3.card, how_round, command_player, command_cpu1, command_cpu2);
                        if(command_cpu3 == 1){//フォールド
                            fold(&cpu3.falled, &cpu3.called);
                        }
                        else if(command_cpu3 == 2){//コール
                            call(&cpu3, challengep, pool);
                        }
                        else if(command_cpu3 == 3){//レイズ
                            raiseBet(&cpu3, &player.called, &cpu1.called, &cpu2.called, challengep, pool);
                            cpu3.total_raise += 1;
                        }
                    }
                }


                else if(parent == 2){
                    
                    if(cpu1.falled == false){
                        printf("CPU1のターン\n");
                        command_cpu1 = Judge(cpu1.card, how_round, command_player, command_cpu2, command_cpu3);
                        if(command_cpu1 == 1){//フォールド
                            fold(&cpu1.falled, &cpu1.called);
                        }
                        else if(command_cpu1 == 2){//コール
                            call(&cpu1, challengep, pool);
                        }
                        else if(command_cpu1 == 3){//レイズ
                            raiseBet(&cpu1, &player.called, &cpu2.called, &cpu3.called, challengep, pool);
                            cpu1.total_raise += 1;
                        }
                    }
                    if(cpu2.falled == false){
                        printf("CPU2のターン\n");
                        command_cpu2 = Judge(cpu2.card, how_round, command_player, command_cpu1, command_cpu3);
                        if(command_cpu2 == 1){//フォールド
                            fold(&cpu2.falled, &cpu2.called);
                        }
                        else if(command_cpu2 == 2){//コール
                            call(&cpu2, challengep, pool);
                        }
                        else if(command_cpu2 == 3){//レイズ
                            raiseBet(&cpu2, &player.called, &cpu1.called, &cpu3.called, challengep, pool);
                            cpu2.total_raise += 1;
                        }
                    }
                    if(cpu3.falled == false){
                        printf("CPU3のターン\n");
                        command_cpu3 = Judge(cpu3.card, how_round, command_player, command_cpu1, command_cpu2);
                        if(command_cpu3 == 1){//フォールド
                            fold(&cpu3.falled, &cpu3.called);
                        }
                        else if(command_cpu3 == 2){//コール
                            call(&cpu3, challengep, pool);
                        }
                        else if(command_cpu3 == 3){//レイズ
                            raiseBet(&cpu3, &player.called, &cpu1.called, &cpu2.called, challengep, pool);
                            cpu3.total_raise += 1;
                        }
                    }

                    right_command = 0;//バグ防止リセット
                    if(player.falled == false){
                        while(right_command==0){//正しいコマンドを入力するまで終わらない用
                        
                            printf("現在の掛けチップ数：%d、現在のチッププール数：%d\n",*challengep,*pool);
                            printf("フォールド=１、コール=２、レイズ=３、カードの確認=４\n");//コールか、レイズか、フォールドか(全員コールで抜けられる)
                            printf("あなたの番です。どうしますか？：");
                        scanf("%d",&command_player);
                        
                        if(command_player == 2 && *challengep == 0){//初めてのコール
                            printf("いくつチップを賭けますか？：");
                            scanf("%d",challengep);
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 3 && *challengep == 0){//レイズ回避用
                            printf("いくつチップを賭けますか？：");
                            scanf("%d",challengep);
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 1){//フォールド
                            fold(&player.falled, &player.called);
                            right_command = 1;
                        }
                        else if(command_player == 2){//コール
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 3){//レイズ
                            raiseBet(&player, &cpu1.called, &cpu2.called, &cpu3.called, challengep, pool);
                            player.total_raise += 1;
                            right_command = 1;
                        }
                        else if(command_player == 4){
                            printf("あなたのカード\n");
                            for(j=0;j<2;j++){
                                printf("%d ", player.card[j]);
                            }
                            printf("\n場のカード\n");
                            for(j=2;j<=how_round+2;j++){
                                printf("%d ", player.card[j]);
                            }
                            printf("\n");
                        }
                        else{
                            printf("エラー。違う数を入力してください。\n");
                        }
                        }
                    }
                }


                else if(parent == 3){
                    
                    if(cpu2.falled == false){
                        printf("CPU2のターン\n");
                        command_cpu2 = Judge(cpu2.card, how_round, command_player, command_cpu1, command_cpu3);
                        if(command_cpu2 == 1){//フォールド
                            fold(&cpu2.falled, &cpu2.called);
                        }
                        else if(command_cpu2 == 2){//コール
                            call(&cpu2, challengep, pool);
                        }
                        else if(command_cpu2 == 3){//レイズ
                            raiseBet(&cpu2, &player.called, &cpu1.called, &cpu3.called, challengep, pool);
                            cpu2.total_raise += 1;
                        }
                    }
                    if(cpu3.falled == false){
                        printf("CPU3のターン\n");
                        command_cpu3 = Judge(cpu3.card, how_round, command_player, command_cpu1, command_cpu2);
                        if(command_cpu3 == 1){//フォールド
                            fold(&cpu3.falled, &cpu3.called);
                        }
                        else if(command_cpu3 == 2){//コール
                            call(&cpu3, challengep, pool);
                        }
                        else if(command_cpu3 == 3){//レイズ
                            raiseBet(&cpu3, &player.called, &cpu1.called, &cpu2.called, challengep, pool);
                            cpu3.total_raise += 1;
                        }
                    }

                    right_command = 0;//バグ防止リセット
                    if(player.falled == false){
                        while(right_command==0){//正しいコマンドを入力するまで終わらない用
                        
                        printf("現在の掛けチップ数：%d、現在のチッププール数：%d\n",*challengep,*pool);
                        printf("フォールド=１、コール=２、レイズ=３、カードの確認=４\n");//コールか、レイズか、フォールドか(全員コールで抜けられる)
                        printf("あなたの番です。どうしますか？：");
                        scanf("%d",&command_player);

                        if(command_player == 2 && *challengep == 0){//初めてのコール
                            printf("いくつチップを賭けますか？：");
                            scanf("%d",challengep);
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 3 && *challengep == 0){//レイズ回避用
                            printf("いくつチップを賭けますか？：");
                            scanf("%d",challengep);
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 1){//フォールド
                            fold(&player.falled, &player.called);
                            right_command = 1;
                        }
                        else if(command_player == 2){//コール
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 3){//レイズ
                            raiseBet(&player, &cpu1.called, &cpu2.called, &cpu3.called, challengep, pool);
                            player.total_raise += 1;
                            right_command = 1;
                        }
                        else if(command_player == 4){
                            printf("あなたのカード\n");
                            for(j=0;j<2;j++){
                                printf("%d ", player.card[j]);
                            }
                            printf("\n場のカード\n");
                            for(j=2;j<=how_round+2;j++){
                                printf("%d ", player.card[j]);
                            }
                            printf("\n");
                        }
                        else{
                            printf("エラー。違う数を入力してください。\n");
                        }
                        }
                    }

                    if(cpu1.falled == false){
                        printf("CPU1のターン\n");
                        command_cpu1 = Judge(cpu1.card, how_round, command_player, command_cpu2, command_cpu3);
                        if(command_cpu1 == 1){//フォールド
                            fold(&cpu1.falled, &cpu1.called);
                        }
                        else if(command_cpu1 == 2){//コール
                            call(&cpu1, challengep, pool);
                        }
                        else if(command_cpu1 == 3){//レイズ
                            raiseBet(&cpu1, &player.called, &cpu2.called, &cpu3.called, challengep, pool);
                            cpu1.total_raise += 1;
                        }
                    }
                }


                else if(parent == 4){
                    
                    if(cpu3.falled == false){
                        printf("CPU3のターン\n");
                        command_cpu3 = Judge(cpu3.card, how_round, command_player, command_cpu1, command_cpu2);
                        if(command_cpu3 == 1){//フォールド
                            fold(&cpu3.falled, &cpu3.called);
                        }
                        else if(command_cpu3 == 2){//コール
                            call(&cpu3, challengep, pool);
                        }
                        else if(command_cpu3 == 3){//レイズ
                            raiseBet(&cpu3, &player.called, &cpu1.called, &cpu2.called, challengep, pool);
                            cpu3.total_raise += 1;
                        }
                    }

                    right_command = 0;//バグ防止リセット
                    if(player.falled == false){
                        while(right_command==0){//正しいコマンドを入力するまで終わらない用
                        
                        printf("現在の掛けチップ数：%d、現在のチッププール数：%d\n",*challengep,*pool);
                        printf("フォールド=１、コール=２、レイズ=３、カードの確認=４\n");//コールか、レイズか、フォールドか(全員コールで抜けられる)
                        printf("あなたの番です。どうしますか？：");
                        scanf("%d",&command_player);

                        if(command_player == 2 && *challengep == 0){//初めてのコール
                            printf("いくつチップを賭けますか？：");
                            scanf("%d",challengep);
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 3 && *challengep == 0){//レイズ回避用
                            printf("いくつチップを賭けますか？：");
                            scanf("%d",challengep);
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 1){//フォールド
                            fold(&player.falled, &player.called);
                            right_command = 1;
                        }
                        else if(command_player == 2){//コール
                            call(&player, challengep, pool);
                            right_command = 1;
                        }
                        else if(command_player == 3){//レイズ
                            raiseBet(&player, &cpu1.called, &cpu2.called, &cpu3.called, challengep, pool);
                            player.total_raise += 1;
                            right_command = 1;
                        }
                        else if(command_player == 4){
                            printf("あなたのカード\n");
                            for(j=0;j<2;j++){
                                printf("%d ", player.card[j]);
                            }
                            printf("\n場のカード\n");
                            for(j=2;j<=how_round+2;j++){
                                printf("%d ", player.card[j]);
                            }
                            printf("\n");
                        }
                        else{
                            printf("エラー。違う数を入力してください。\n");
                        }
                        }
                    }

                    if(cpu1.falled == false){
                        printf("CPU1のターン\n");
                        command_cpu1 = Judge(cpu1.card, how_round, command_player, command_cpu2, command_cpu3);
                        if(command_cpu1 == 1){//フォールド
                            fold(&cpu1.falled, &cpu1.called);
                        }
                        else if(command_cpu1 == 2){//コール
                            call(&cpu1, challengep, pool);
                        }
                        else if(command_cpu1 == 3){//レイズ
                            raiseBet(&cpu1, &player.called, &cpu2.called, &cpu3.called, challengep, pool);
                            cpu1.total_raise += 1;
                        }
                    }
                    if(cpu2.falled == false){
                        printf("CPU2のターン\n");
                        command_cpu2 = Judge(cpu2.card, how_round, command_player, command_cpu1, command_cpu3);
                        if(command_cpu2 == 1){//フォールド
                            fold(&cpu2.falled, &cpu2.called);
                        }
                        else if(command_cpu2 == 2){//コール
                            call(&cpu2, challengep, pool);
                        }
                        else if(command_cpu2 == 3){//レイズ
                            raiseBet(&cpu2, &player.called, &cpu1.called, &cpu3.called, challengep, pool);
                            cpu2.total_raise += 1;
                        }
                    }
                    
                }

                //全員がコールまたはフォールドしていたら次のラウンドへ
                if(player.called == true || player.falled == true){
                    if(cpu1.called == true || cpu1.falled == true){
                        if(cpu2.called == true || cpu2.falled == true){
                            if(cpu3.called == true || cpu3.falled == true){
                                all_call = 1;
                            }
                        }
                    }
                }
                //printf("現在のall_callは%d\n",all_call);//デバッグ用


            }
        }

        //cpuはエラー回避用
        sort(player.card, 5);
        sort(cpu1.card, 5);
        sort(cpu2.card, 5);
        sort(cpu3.card, 5);

        
        //ショーダウン
        printf("あなたの手札は\n");
        for(j=0;j<=4;j++)
        {
            printf("%d枚目：%d　",j+1,player.card[j]);
        }
        get_point[0] = showdown(player.card);//獲得ポイントの計算
        printf("\ncpu1の手札は\n");
        for(j=0;j<=4;j++)
        {
            printf("%d枚目：%d　",j+1,cpu1.card[j]);
        }
        get_point[1] = showdown(cpu1.card);//獲得ポイントの計算
        printf("\ncpu2の手札は\n");
        for(j=0;j<=4;j++)
        {
            printf("%d枚目：%d　",j+1,cpu2.card[j]);
        }
        get_point[2] = showdown(cpu2.card);//獲得ポイントの計算
        printf("\ncpu3の手札は\n");
        for(j=0;j<=4;j++)
        {
            printf("%d枚目：%d　",j+1,cpu3.card[j]);
        }
        get_point[3] = showdown(cpu3.card);//獲得ポイントの計算

        be_fold[0] = player.falled;
        be_fold[1] = cpu1.falled;
        be_fold[2] = cpu2.falled;
        be_fold[3] = cpu3.falled;

        //foldを選択したものはget_pointを0にする
        //先に処理するのは、視覚的に勝利者が分かりやすくするため
        who_winner = Who_round_winner(get_point, be_fold);

        //比較結果の処理
        printf("\nあなたの獲得ポイントは%d、cpu1の獲得ポイントは%d\ncpu2の獲得ポイントは%d、cpu3の獲得ポイントは%d\n",get_point[0],get_point[1],get_point[2],get_point[3]);
        my_sleep(4);

        //勝敗結果処理
        if(who_winner == 1){
            printf("このラウンドは、あなたの勝ち\n");
            player.cip += *pool;
        }
        else if(who_winner == 2){
            printf("このラウンドは、cpu1の勝ち\n");
            cpu1.cip += *pool;
        }
        else if(who_winner == 3){
            printf("このラウンドは、cpu2の勝ち\n");
            cpu2.cip += *pool;
        }
        else if(who_winner == 4){
            printf("このラウンドは、cpu3の勝ち\n");
            cpu3.cip += *pool;
        }
        else{
            printf("このラウンドは、引き分け\n");
            player.cip += *pool;
            cpu1.cip += *pool;
            cpu2.cip += *pool;
            cpu3.cip += *pool;
        }
        my_sleep(2);
        
    }

    printf("獲得チップの合計\n");
    printf("あなた：%d、cpu1：%d、cpu2：%d、cpu3：%d\n",player.cip, cpu1.cip, cpu2.cip, cpu3.cip);
    my_sleep(2);
    //チップの集計処理(別のサブ関数を用意する)
    who_winner = Who_game_winner(player.cip, cpu1.cip, cpu2.cip, cpu3.cip);
    if(who_winner == 1)//ゲーム自体は一番出ないと負けとする。
    {
        printf("このゲームは、あなたの勝ち！！\n");
    }
    else{

        printf("このゲームは、あなたの負け・・・\n");
    }
}

void getcard(int all_card[], int card[], int size){//カードを配るための関数
    int i,j;

    for(i=0;i<size;i++){
        j = rand()%52;//0～51を指定
        while(all_card[j] == 0){//既に配られた場合は再抽選
            j = rand()%52;
        }
        card[i] = all_card[j];
        //printf("%dをゲット\n",card[i]);//デバッグ用
        all_card[j] = 0;
    }
}

void sort(int subject_card[], int size){//カードを昇順に入れ替えるための関数
    int keep_card,i,j;
    for(i=0;i<size-1;i++)
        {
            for(j=i+1;j<size;j++)
            {
                if(subject_card[j]<subject_card[i])
                {
                    keep_card = subject_card[i];
                    subject_card[i] = subject_card[j];
                    subject_card[j] = keep_card;
                }
            }
        }
}

int Toparent(int p_card[], int c1_card[], int c2_card[], int c3_card[]){//親決め関数(J持ちが親)
    int Ans = 0;
    if(p_card[0] == 11){//どちらかのカードがJなら親になる
        Ans = 1;
    }
    else if(c1_card[0] == 11){
        Ans = 2;
    }
    else if(c2_card[0] == 11){
        Ans = 3;
    }
    else if(c3_card[0] == 11){
        Ans = 4;
    }
    else if(p_card[1] == 11){
        Ans = 1;
    }
    else if(c1_card[1] == 11){
        Ans = 2;
    }
    else if(c2_card[1] == 11){
        Ans = 3;
    }
    else if(c3_card[1] == 11){
        Ans = 4;
    }
    else{//誰もいなかったらplayerが親になる
        Ans = 1;
    }

    printf("今回の親はプレイヤー%dです。\n",Ans);
    return Ans;
}

int Judge(int c_card[], int show_number, int judge1, int judge2, int judge3){
    //現在渡されている引数は自分のカードとラウンド数。
    int i;//繰り返し用変数
    int judge;//戻り値用の変数
    int thought_point = 0;//予想できる獲得ポイント(役の強さ)
    int or_raise1,or_raise2,or_raise3;//レイズしたかどうか
    //int dealer_card[1] = c_card[2];//場のカード参照用？

    //ラウンドが3つに対して、カードは5枚あるため。
    show_number += 2;
    or_raise1 = or_raise2 = or_raise3 = 0;

    //c_cardが判断元のカードの内容、showはディーラーの開示数(ラウンド数)
    sort(c_card, show_number);

    //最低3カードある最大5カード 判断する組み合わせ：○○○●● ○○○○● ○○○○○
    //ラウンド数によって考える
    if(show_number == 2){thought_point = thought3(c_card);}
    else if(show_number == 3){thought_point = thought4(c_card);}
    else{thought_point = thought5(c_card);}

    if(judge1 == 3){or_raise1 = 1;}
    if(judge2 == 3){or_raise2 = 1;}
    if(judge3 == 3){or_raise3 = 1;}

    //レイズした時、ラウンド数が高いとより強い手だと判断させる。
    thought_point = thought_point - ((or_raise1*show_number) + (or_raise2*show_number) + (or_raise3*show_number));
    
    //最高値：78+13=91、最低値：13
    //最低レイズ予定　3カード確定の最低値：39+1=40
    //最高値と最低値の1/4と3/4の値を用いる：19、58
    //最高フォールド予定 ハイカードの最高値：13
    if(thought_point <= 19){//判断：フォールド
        judge = 1;
    }
    else if(thought_point <= 58){//判断：コール
        judge = 2;
    }
    else{//判断：レイズ
        judge = 3;
    }
    
    my_sleep(3);
    return judge;
    
}

//思ったよりフォールドしやすいため、もう少しコールさせる
//まず上手く考えられているのか見てみる
//ボーナススコア　ワンペア：+13、ツーペア：+26、3カード：+39、ストレート：+52、フルハウス：+65、4カード：+78
int thought3(int card[]){//ラウンド1　持っているカード数：3枚、場のカード数：1枚
    //考慮すべき組み合わせ：ハイカード、2ペア、1ペア、3|4カード、(フルハウス)、(ストレート)
    //確定する可能性：1ペア、3カード、ハイカード
    int getpoint;

    //リセット用
    getpoint = 0;

    //確定フェーズ
    if(card[0] == card[1]){//ペア確定
        if(card[0] == card[2]){//3カード
            getpoint = getpoint + 39 + card[2];
        }
        else{
            getpoint = getpoint + 13 + card[1];
        }
    }
    else if(card[1] == card[2]){//後半1ペア
        getpoint = getpoint + 13 + card[2];
    }
    else{//ハイカード
        getpoint = getpoint + card[2];
    }

    //想定フェーズ※とりあえずスコアは半分の加算にする
    if(card[0] == card[1]){
        if(card[0] == card[2]){//3:2フルハウス・4カードの可能性
            getpoint += 71;
        }
        else{//2ペアの可能性
            getpoint += 13;
        }
    }
    else if(card[1] == card[2]){//3|4カードの可能性
        getpoint += 19;
    }
    else if(card[0]+1 == card[1]){
        if(card[1]+1 == card[2]){//ストレートの可能性
            getpoint += 26;
        }
    }

    //printf("期待できるポイントは%d",getpoint);
    return getpoint;

}

int thought4(int card[]){//ラウンド2　持っているカード数：4枚、場のカード数：2枚
    //考慮すべき組み合わせ：ハイカード、2ペア、1ペア、3|4カード、(フルハウス)、(ストレート)
    //確定する可能性：1ペア、2ペア、3カード、ハイカード、4カード
    int getpoint;

    //リセット用
    getpoint = 0;

    //確定フェーズ
    if(card[0] == card[1]){//ペア確定
        if(card[0] == card[2]){//3カード
            if(card[0] == card[3]){//4カード
                getpoint = getpoint + 78 + card[3];
            }
            else{
                getpoint = getpoint + 39 + card[2];
            }
        }
        else if(card[2] == card[3]){//2ペア
            getpoint = getpoint + 26 + card[3];
        }
        else{
            getpoint = getpoint + 13 + card[1];
        }
    }
    else if(card[1] == card[2]){//後半1ペア
        if(card[2] == card[3]){//3カード
            getpoint = getpoint + 39 + card[3];
        }
        else{
            getpoint = getpoint + 13 + card[2];
        }
    }
    else if(card[2] == card[3]){
        getpoint = getpoint + 13 + card[3];
    }
    else{//ハイカード
        getpoint = getpoint + card[3];
    }

    //想定フェーズ※とりあえずスコアは半分の加算にする
    if(card[0] == card[1]){
        if(card[0] == card[2]){
            if(card[2] != card[3]){//3:2フルハウスの可能性
                getpoint += 32;
            }
        }
        else if(card[2] == card[3]){//2:3フルハウスの可能性
            getpoint += 32;
        }
        else{//2ペアの可能性
            getpoint += 13;
        }
    }
    else if(card[1] == card[2]){//4カードの可能性
        if(card[2] == card[3]){
            getpoint += 39;
        }
        else{//2ペアの可能性
            getpoint +- 13;
        }
    }
    else if(card[2] == card[3]){//3カードの可能性
        getpoint += 19;
    }
    else if(card[0]+1 == card[1]){
        if(card[1]+1 == card[2]){
            if(card[2]+1 == card[3]){//ストレートの可能性
                getpoint += 26;
            }
        }
    }

    //printf("期待できるポイントは%d",getpoint);
    return getpoint;

}

int thought5(int card[]){//ラウンド3　持っているカード数：5枚、場のカード数：3枚
    int getpoint;

    //リセット用
    getpoint = 0;

    //確定フェーズ
    if(card[0] == card[1]){//ペア
        if(card[1] == card[2]){//3
            if(card[2] == card[3]){//4
                getpoint = 78 + card[3];//4カード
            }
            else if(card[3] == card[4]){//フルハウス3:2
                getpoint = 65 + card[2];
            }
            else{
                getpoint = 39 + card[2];
            }//
        }
        else if(card[2] == card[3] && card[3] == card[4]){
            getpoint = 65 + card[4];
        }//フルハウス2:3
        else if(card[2] == card[3])
        {
            getpoint = 26 + card[3];
        }
        else if(card[3] == card[4])//離れたツーペア処理
        {
            getpoint = 26 + card[4];
        }
        else{
            getpoint = 13 + card[1];
        }//1ペア
    }

    else if(card[1] == card[2]){//ペア
        if(card[2] == card[3]){//3
            if(card[3] == card[4]){//4
                getpoint = 78 + card[4];
            }
            else{
                getpoint = 39 + card[3];
            }
        }
        else if(card[3] == card[4]){
            getpoint = 26 + card[4];
        }
        else{
            getpoint = 13 + card[2];
        }
    }

    else if(card[2] == card[3]){//ペア
        if(card[3] == card[4]){
            getpoint = 39 + card[4];
        }
        else{
            getpoint = 13 + card[3];
        }
    }

    else if(card[3] == card[4]){
        getpoint = 13 + card[4];
    }

    else if(card[0]+1 == card[1]){//ストレート
        if(card[1]+1 == card[2]){
            if(card[2]+1 == card[3]){
                if(card[3]+1 == card[4]){
                    getpoint = 52 + card[4];   
                }
                else{
                    getpoint = card[4];
                }
            }
            else{
                getpoint = card[4];
            }
        }
        else{
            getpoint = card[4];
        }
    }
    else{//ブタ
        getpoint = card[4];
    }

    return getpoint;

}

void call(struct Playing *player, int *bet_cip, int *pool){
    player->cip -= *bet_cip;
    *pool += *bet_cip;

    printf("コマンド：コール\n");
    player->called = true;
}

void raiseBet(struct Playing *player, bool *called1, bool *called2, bool *called3, int *bet_cip, int *pool){
    *bet_cip = *bet_cip * 2;
    player->cip -= *bet_cip;
    *pool += *bet_cip;

    printf("コマンド：レイズ\n");
    player->called = true;
    *called1 = false;
    *called2 = false;
    *called3 = false;
}

void fold(bool *fold, bool *call){
    *fold = true;
    *call = true;
    printf("コマンド：フォールド\n");
}

//ボーナススコア　ワンペア：+13、ツーペア：+26、3カード：+39、ストレート：+52、フルハウス：+65、4カード：+78
int showdown(int score[5])//ポーカーの出目を比較する
{
    int getpoint=0;
    if(score[0]==score[1]){//ペア
        if(score[1]==score[2]){//3
            if(score[2]==score[3]){//4
                printf("4カード\n");
                getpoint = 78 + score[3];//4カード
            }
            else if(score[3]==score[4]){//フルハウス3:2
                printf("フルハウス\n");
                getpoint = 65 + score[2];
            }
            else{
                printf("3カード\n");
                getpoint = 39 + score[2];
            }//
        }
        else if(score[2]==score[3]&&score[3]==score[4]){
            printf("フルハウス\n");
            getpoint = 65 + score[4];
        }//フルハウス2:3
        else if(score[2]==score[3])
        {
            printf("2ペア\n");
            getpoint = 26 + score[3];
        }
        else if(score[3]==score[4])//離れたツーペア処理
        {
            printf("2ペア\n");
            getpoint = 26 + score[4];
        }
        else{
            printf("1ペア");
            getpoint = 13 + score[1];
        }//1ペア
    }

    else if(score[1]==score[2]){//ペア
        if(score[2]==score[3]){//3
            if(score[3]==score[4]){//4
                printf("4カード\n");
                getpoint = 78 + score[4];
            }
            else{
                printf("3カード\n");
                getpoint = 39 + score[3];
            }
        }
        else if(score[3]==score[4]){
            printf("2ペア\n");
            getpoint = 26 + score[4];
        }
        else{
            printf("1ペア\n");
            getpoint = 13 + score[2];
        }
    }

    else if(score[2]==score[3]){//ペア
        if(score[3]==score[4]){
            printf("3カード\n");
            getpoint = 39 + score[4];
        }
        else{
            printf("1ペア\n");
            getpoint = 13 + score[3];
        }
    }

    else if(score[3]==score[4]){
        printf("1ペア\n");
        getpoint = 13 + score[4];
    }

    else if(score[0]+1==score[1]){//ストレート
        if(score[1]+1==score[2]){
            if(score[2]+1==score[3]){
                if(score[3]+1==-score[4]){
                    printf("ストレート\n");
                    getpoint = 52 + score[4];   
                }
                else{
                    printf("ハイカード\n");
                    getpoint = score[4];
                }
            }
            else{
                printf("ハイカード\n");
                getpoint = score[4];
            }
        }
        else{
            printf("ハイカード\n");
            getpoint = score[4];
        }
    }
    else{//ブタ
        printf("ハイカード\n");
        getpoint = score[4];
    }

    return getpoint;
}

int Who_round_winner(int Get_point[4], bool Be_fold[4]){

    int winner=0;
    int i;

    for(i=0;i<4;i++){
        if(Be_fold[i] == true){
            Get_point[i] = 0;
        }
    }

    if(Get_point[0] > Get_point[1]){//1>2
        if(Get_point[0] > Get_point[2]){//1>2,3
            if(Get_point[0] > Get_point[3]){//1>2,3,4
                winner = 1;
            }
            else{//4>1,2,3
                winner = 4;
            }
        }
        else{
            if(Get_point[2] > Get_point[3]){//3>1,2,4
                winner = 3;
            }
            else{//4>1,2,3
                winner = 4;
            }
        }
    }
    else if(Get_point[1] > Get_point[2]){//2>1,3
        if(Get_point[1] > Get_point[3]){//2>1,3,4
            winner = 2;
        }
        else{//4>1,2,3
            winner = 4;
        }
    }
    else if(Get_point[2] > Get_point[3]){//3>1,2,4
        winner = 3;
    }
    else if(Get_point[3] > Get_point[2]){//4>1,2,3
        winner = 4;
    }
    else{//引き分け※全員foldした場合
        winner = 5;
    }

    return winner;
}

int Who_game_winner(int point1, int point2, int point3, int point4){

    int winner=0;

    if(point1 > point2){//1>2
        if(point1 > point3){//1>2,3
            if(point1 > point4){//1>2,3,4
                winner = 1;
            }
            else{//4>1,2,3
                winner = 4;
            }
        }
        else{
            if(point3 > point4){//3>1,2,4
                winner = 3;
            }
            else{//4>1,2,3
                winner = 4;
            }
        }
    }
    else if(point2 > point3){//2>1,3
        if(point2 > point4){//2>1,3,4
            winner = 2;
        }
        else{//4>1,2,3
            winner = 4;
        }
    }
    else if(point3 > point4){//3>1,2,4
        winner = 3;
    }
    else if(point4 > point3){//4>1,2,3
        winner = 4;
    }
    else{//引き分け※ないと思う
        winner = 5;
    }

    return winner;
}

void my_sleep(unsigned int seconds) {
#ifdef _WIN32
    Sleep(seconds * 1000);  // milliseconds
#else
    sleep(seconds);         // seconds
#endif
}