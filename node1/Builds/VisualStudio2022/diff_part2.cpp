#include <iostream>
#include<string>
std::string IN("INPUT_bin.txt");
std::string OUT("project2_bits_receiver.txt");
int main(){
FILE *file_in=fopen(IN.c_str(),"r");
FILE *file_out=fopen(OUT.c_str(),"r");
FILE *diff=fopen("diff_project_2.txt","w");
int count = 0;
int count_same = 0;
while(1){
    count++;
    char num1,num2;
    int break_efo1=fscanf(file_in,"%c",&num1);
int break_efo2=fscanf(file_out,"%c",&num2);
if(break_efo1==EOF||break_efo2==EOF){
    break;
}
fprintf(diff,"%d",num1!=num2);
if (num1 == num2) {
    count_same++;
}
}
fprintf(diff, " count_same:%f", count_same * 1.0 / count);
printf("the same_rate is %f", count_same * 1.0 / count);
fclose(file_in);
fclose(file_out);
fclose(diff);
return 0;


}