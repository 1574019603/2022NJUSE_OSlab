#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

extern "C"{
    void asm_printRed(const char* str, int len);
    void asm_printNormal(const char* str, int len);
}

void printRed(const string& str){
    asm_printRed(str.c_str(),(int)str.length());
}

void printNormal(const string& str){
    asm_printNormal(str.c_str(),(int)str.length());
}

void AnotherLine(){
    char c = 0xa;
    asm_printNormal(&c,1);
}


//簇类，是FAT12的基本组织
class cluster{
public:
    char content[512]{};

    explicit cluster(const char* b){
        for(int i = 0;i<512;i++){
            content[i] = b[i];
        }
    }
};


//先定义引导扇区
class Boot_Sector{
public:
    //先定义开头512字节的信息,BPB代表数据
    char boot_jmp[3]{};
    char boot_OEM[8]{};
    char BPB_BytesPerSection[2]{};//每个扇区的字节数，默认为512
    char BPB_SectionPerCluster{};//每个簇的扇区数目
    char BPB_ReservedSec[2]{};//bootrecord占用扇区数
    char BPB_NumOfFAT{};//FAT数目，一般为2
    char BPB_NumOfRootDirectoryEnt[2]{};//根目录文件数
    char BPB_NumOfSec1[2]{};//扇区数
    char BPB_Media{};//介质描述符
    char BPB_SizePerFAT[2]{};//每个FAT表所占扇区数
    char BPB_SectionPerTrack[2]{};//每个磁道所占扇区数
    char BPB_NumOfHeads[2]{};//磁头数目
    char BPB_HiddenSecs[4]{};//隐藏的扇区数目
    char BPB_NumOfSec2[4]{};//如果BPB_NumOfSec1= 0，则由此代表扇区数目
    char boot_DriveNum{};//INT 13H驱动器号
    char boot_Reserved1{};//保留未使用
    char boot_BootSign{};//扩扎努引导记号
    char boot_VolumeID[4]{};//卷序列号
    char boot_volumeSign[11]{};//卷标记
    char boot_FSType[8]{};//文件系统类型，本次试验为"FAT12"
    char boot_anotherCode[448]{};
    char EndSign[2]{};//结束标志

    //默认构造函数
    Boot_Sector() = default;

    //有参数构造函数
    Boot_Sector(cluster c){
        //按照给定扇区初始化各项
        int index = 0;
        for(char& i:boot_jmp){
            i = c.content[index];
            index++;
        }
        for(char& i:boot_OEM){
            i = c.content[index];
            index++;
        }
        for(char& i:BPB_BytesPerSection){
            i = c.content[index];
            index++;
        }
        BPB_SectionPerCluster = c.content[index];
        index++;

        for(char& i:BPB_ReservedSec){
            i = c.content[index];
            index++;
        }
        BPB_NumOfFAT = c.content[index];
        index++;

        for(char& i:BPB_NumOfRootDirectoryEnt){
            i = c.content[index];
            index++;
        }

        for(char& i:BPB_NumOfSec1){
            i = c.content[index];
            index++;
        }

        BPB_Media = c.content[index];
        index++;
        for(char& i:BPB_SizePerFAT){
            i = c.content[index];
            index++;
        }

        for(char& i:BPB_SectionPerTrack){
            i = c.content[index];
            index++;
        }

        for(char& i:BPB_NumOfHeads){
            i = c.content[index];
            index++;
        }
        for(char& i:BPB_HiddenSecs){
            i = c.content[index];
            index++;
        }
        for(char& i:BPB_NumOfSec2){
            i = c.content[index];
            index++;
        }
        boot_DriveNum = c.content[index];
        index++;
        boot_Reserved1 = c.content[index];
        index++;
        boot_BootSign = c.content[index];
        index++;
        for(char& i:boot_VolumeID){
            i = c.content[index];
            index++;
        }
        for(char& i:boot_volumeSign){
            i = c.content[index];
            index++;
        }
        for(char& i:boot_FSType){
            i = c.content[index];
            index++;
        }
        for(char& i:boot_anotherCode){
            i = c.content[index];
            index++;
        }
        for(char& i:EndSign){
            i = c.content[index];
            index++;
        }
    }
};

//再定义FAT
class FAT_table{
private:
    vector<unsigned int> clusts;//类似链表的形式
    char content[9*512]{};//一个FAT占9个扇区
    int addnum = 0;
public:
    void addCluster(cluster c);//用于加载一个扇区进入fat表，一共需要加载9个

    void createTheClusts();//用于加载链表结构

    int getNext(int index){
        if (clusts[index]<2 || clusts[index]>clusts.size()){
            return -1;
        }

        return clusts[index];
    }

};

void FAT_table::addCluster(cluster c) {
    for(int i = 0;i<512;i++){
        content[i+addnum*512] = c.content[i];
    }
    addnum++;
}

void FAT_table::createTheClusts() {
    if (addnum < 9){
        cout<<"less than 9 cluster"<<endl;
    } else {
        int index = 0;
        while (index < 512*9){
            //因为一个FAT entry占12位，所以一次性读取2个entry，即3个字节
            unsigned char bytes[3];
            for(unsigned char& i:bytes){
                i = (unsigned char)content[index];
                index++;
            }

            //接下来存12位，注意小端存储,first是第二个的后4位加第一个
            unsigned int first = (((unsigned int)(bytes[1])%16)<<8) + (unsigned int)(bytes[0]);
            unsigned int second = (unsigned int)(bytes[2]<<4) + (unsigned int)(bytes[1]>>4);
            clusts.push_back(first);
            clusts.push_back(second);
        }
    }
}

//根目录项，是根目录的组成单位
class Directory_Entry{
public:
    char Directory_Name[11]{};//文件名
    char Directory_Attribute{};
    char Reserved[10]{};
    char Directory_WriteTime[2]{};//更改时间和日期
    char Directory_WriteDate[2]{};
    char Directory_FATID[2]{};//对应的FAT项号
    char Directory_FileSize[4]{};//文件大小
    Directory_Entry():Directory_Attribute(' '){}

    Directory_Entry(const char* directory, int start){
        int index = start;
        for(char& i:Directory_Name){
            i = directory[index];
            index++;
        }
        Directory_Attribute = directory[index];
        index++;
        for(char& i:Reserved){
            i = directory[index];
            index++;
        }
        for(char& i:Directory_WriteTime){
            i = directory[index];
            index++;
        }
        for(char& i:Directory_WriteDate){
            i = directory[index];
            index++;
        }
        for(char& i:Directory_FATID){
            i = directory[index];
            index++;
        }
        for(char& i:Directory_FileSize){
            i = directory[index];
            index++;
        }
    }

};
//根目录
class Root_Directory{
private:
    int addnum = 0;
    char* content{};//由于根目录区大小并不知道所以需要通过引导扇区确定，故需要通过main函数来实现
    int size = 0;
public:
    vector<Directory_Entry*> entrys;//存放目录项
    Root_Directory() = default;
    void setSizeAndContent(int size);
    void addCluster(cluster& c);//构建content
    void createEntryList();//根据content构建根目录表


};

void Root_Directory::setSizeAndContent(int size) {
    this->size = size;
    content = new char[512*((size*32+511)/512)];//为什么加511？最后一个entry独占一个扇区
}

void Root_Directory::addCluster(cluster &c) {
    for(int i = 0;i<512;i++){
        content[i+addnum*512] = c.content[i];
    }
    addnum++;
}

void Root_Directory::createEntryList() {
    for(int i = 0;i<size;i++){
        auto* entry = new Directory_Entry(content,i*32);
        if((int)entry->Directory_Name[0] != 0&&(int)entry->Directory_Attribute!= 15){
            entrys.push_back(entry);
        }
    }
}

//文件类
class FAT_File{
private:
    string name;
    string content;

public:
    FAT_File() = default;
    FAT_File(FAT_table fat, Directory_Entry entry, vector<cluster*>& files, int start):name(entry.Directory_Name,11){
        int fatTableId = (int)((unsigned char)entry.Directory_FATID[0] + (unsigned char)entry.Directory_FATID[1]*256);
        while (fatTableId != -1){
            //start指数据区起始扇区号，-2指前两个扇区是无效的
            int index = start + (fatTableId-2);
            content.append(files[index]->content,512);
            fatTableId = fat.getNext(fatTableId);
        }
    }

    string getName(){
        string res;
        int i = 7,j = 10;
        //去除空格
        for(;i>=0;i--){
            if (name[i]!=' '){
                break;
            }
        }

        for (;j>=8;j--){
            if (name[j]!= ' '){
                break;
            }
        }
        res.append(name.substr(0,i+1));
        res.append(".");
        res.append(name.substr(8,j-7));
        return res;
    }
    string getContent(){
        string res;
        int len = content.length()-1;
        for(;len>=0;len--){
            if (content[len]!=0){
                break;
            }
        }
        res.append(content.substr(0,len+1));
        return res;
    }
};
//目录
class FAT_Directory{
private:
    string name;
    vector<FAT_File*> files;
    vector<FAT_Directory*> subDirectorys;

public:
    //三种构造函数
    FAT_Directory() = default;
    FAT_Directory(Directory_Entry entry,FAT_table fat, vector<cluster*> imgFile, int start):name(entry.Directory_Name,11){
        int fatTableId = (int)((unsigned char)entry.Directory_FATID[0] + (unsigned char)entry.Directory_FATID[1]*256);

        while(fatTableId!=-1){
            int index = start+fatTableId-2;
            //目录项大小为32
            for (int i = 0; i < 512; i+=32) {
                Directory_Entry entry2(imgFile[index]->content,i);
                //判断是目录还是文件
                if (entry2.Directory_Attribute == 16&&entry2.Directory_Name[0]!='.'){
                    auto sybdir = new FAT_Directory(entry2,fat,imgFile,start);
                    if(sybdir->getName().find("~1")== string::npos){
                        subDirectorys.push_back(sybdir);
                    }
                } else if (entry2.Directory_Attribute == 32 || (entry2.Directory_Attribute == 0&&entry2.Directory_Name[0]!=0)){
                    auto subfile = new FAT_File(fat,entry2,imgFile,start);
                    if(subfile->getName().find("~1")== string::npos){
                        files.push_back(subfile);
                    }
                }
            }
            fatTableId = fat.getNext(fatTableId);
        }
    }
    FAT_Directory(vector<FAT_File*> subFs,vector<FAT_Directory*> subDs,string name){
        this->files = std::move(subFs);
        this->subDirectorys =  std::move(subDs);
        this->name = std::move(name);
    }

    //get方法
    string getName(){
        if (name.empty()) return "";
        string res;
        int i = 7;
        int j = 10;
        for(;i>=0;i--){
            if(name[i]!=' '){
                break;
            }
        }
        for(;j>=8;j--){
            if(name[j]!=' '){
                break;
            }
        }
        res.append(name.substr(0,i+1));
        res.append(name.substr(8,j-7));
        return res;
    }

    vector<FAT_File*>& getFiles(){
        return files;
    }
    vector<FAT_Directory*>& getSubDirectorys(){
        return subDirectorys;
    }

    //展示函数
    //输出子目录项的目录数和文件数或子文件的大小
    void printNum(){
        //  名字空代表为根目录
        if (!name.empty()){
            printRed(".");
            AnotherLine();
            printRed("..");
            AnotherLine();
        }
        //输出子目录相关
        for (FAT_Directory* dir:subDirectorys){
            printRed(dir->getName());
            printNormal(" ");
            printNormal(to_string(dir->subDirectorys.size()));
            printNormal(" ");
            printNormal(to_string(dir->files.size()));
            AnotherLine();
        }

        for (FAT_File* f:files){
            printNormal(f->getName());
            printNormal(" ");
            printNormal(to_string(f->getContent().size()));
            AnotherLine();
        }
    }

    void print(){
        if(!name.empty()){
            printRed(".  ..  ");
        }
        for (FAT_Directory* dir:subDirectorys){
            if(dir->getName().find("~1")!= string::npos) continue;
            printRed(dir->getName());
            printNormal("  ");
        }
        for (FAT_File* f:files){
            printNormal(f->getName());
            printNormal("  ");
        }
    }
    //搜索函数
    FAT_Directory* searchDirByName(const string& thename){
        for(FAT_Directory* dir:subDirectorys){
            if(dir->getName() == thename){
                return dir;
            }
        }
        return nullptr;
    }

    FAT_File* searchFileByName(const string& thename){
        for (FAT_File* f:files){
            if (f->getName() == thename){
                return f;
            }
        }
        return nullptr;
    }

};


//FAT12文件系统类
class FAT12{
private:
    Boot_Sector head;
    FAT_table fat1,fat2;
    Root_Directory rootDirectory;
    vector<cluster*> imgFile;//所有簇
    vector<FAT_File*> files;
    vector<FAT_Directory*> dirs;
    string address;

public:
    FAT_Directory root;
    int dataArea_start = 0;
    void setAddress(string ad){
        address = std::move(ad);
    }
    void load();//加载img所有簇到imgFile
    void init();//初始化FAT12各个部分
    void readFile();
    void readDir();
};

void executeCAT(string basicString, FAT12 fat12);

void FAT12::load() {
    ifstream img(address,ios::binary);
    if(!img.is_open()){
        cout<<"fail to open";
    }

    char buffer[512];
    while(!img.eof()){
        img.read(buffer,512);
        auto* temp = new cluster(buffer);
        imgFile.push_back(temp);
    }

}
void FAT12::init() {
    Boot_Sector boot(*imgFile[0]);
    head = boot;//初始化引导扇区
    //初始化fat表
    for (int i = 1; i <= 9; i++) {
        fat1.addCluster(*imgFile[i]);
    }
    for (int i = 10; i <=18 ; i++) {
        fat2.addCluster(*imgFile[i]);
    }
    fat1.createTheClusts();
    fat2.createTheClusts();
    //初始化根目录区
    int size = (int)((unsigned char)(head.BPB_NumOfRootDirectoryEnt[0])+(unsigned char)(head.BPB_NumOfRootDirectoryEnt[1])*256);
    rootDirectory.setSizeAndContent(size);
    for (int i = 19; i <(int)(19+(size*32+511)/512) ; i++) {
        rootDirectory.addCluster(*imgFile[i]);
    }
    rootDirectory.createEntryList();
    //标记数据区的起始位置
    dataArea_start = (int)(19+(size*32+511)/512);
    //创建根目录内容
    readFile();
    readDir();
    root = *new FAT_Directory(files,dirs,"");

}

void FAT12::readFile() {
    for(Directory_Entry* ent:rootDirectory.entrys){
        if(ent->Directory_Attribute==32 || (ent->Directory_Attribute==0&&ent->Directory_Name[0]!=0)){
            auto* file = new FAT_File(this->fat1,*ent, this->imgFile, this->dataArea_start);
            if (file->getName().find("~1") == string::npos){
                files.push_back(file);
            }
        }
    }
}

void FAT12::readDir() {
    for(Directory_Entry* ent:rootDirectory.entrys){
        if(ent->Directory_Attribute == 16){
            auto* directory = new FAT_Directory(*ent, this->fat1,this->imgFile,this->dataArea_start);
            if (directory->getName().find("~1") == string::npos){
                dirs.push_back(directory);
            }
        }
    }
}

//判断
bool isPath(const string& args,int idx){
    for(int i=idx;i<args.size();i++){
        if(args[i]=='-'){
            break;
        }
        else if(args[i]!=' '){
            return false;
        }
    }
    return true;
}


//获取路径
string getPathString(const string& args){
    if(args.empty()) return "";
    if(args.length() == 1) return args;

    string res;
    int index = 0;
    int len = 0;
    bool flag = false;
    for(int i = 0;i<args.length();i++){
        if (args[i]=='-'){
            if (isPath(args,i+1)){
                return "-";
            }
        }
        if (args[i] == ' '&&args[i+1]!='-'&&args[i+1]!=' '){
            index = i+1;
            flag = true;
        }
        if(i==0&&args[i]!='-'&&args[i+1]!=' '){
            index = i;
            flag = true;
        }
    }
    if(!flag) return "";
    for(int i = index;i<args.length();i++){
        if(args[i]!=' '){
            len++;
        }
        else{
            break;
        }
    }
    res = args.substr(index,len);
    return res;
}

bool isAllSpace(const string& s){
    if(s.empty()) return true;
    for(char c:s){
        if(c!=' ') return false;
    }
    return true;
}

//检查命令是否有效
int isValid(string& args){
    if (args.empty()){
        return 1;//代表是ls
    }

    int n = 0;
    int m = 0;
    bool hasL = false;
    for (char arg:args){
        if (arg == '-'){
            if (n == 0){
                n = 1;
            } else{
                return 0;//代表输入无效
            }
        } else if(arg == ' '){
            n = 0;
        } else{
            if (n==0){
                continue;
            } else if(arg == 'l'){
                hasL = true;
            } else{
                return 0;//无效输入
            }
        }
    }
    //接下来看重复地址
    for (int i = 0; i <args.length()-1 ; i++) {
        if((args[i]==' '&&args[i+1]!='-'&&args[i+1]!=' ') || (i == 0&&args[i]!='-'&&args[i+1]!=' '&&args[i+1]!='-')){
            m++;//记录地址个数
            if(m>1) return 3;//代表重复地址
        }
    }
    return hasL?2:1;
}

//带数字的打印
void printWithNum(string name,FAT_Directory* dir){
    if(name[name.size()-1]!='/'){
        name.append("/");
    }
    name.append(dir->getName());
    printNormal(name);
    printNormal(" ");
    printNormal(to_string(dir->getSubDirectorys().size()));
    printNormal(" ");
    printNormal(to_string(dir->getFiles().size()));
    printNormal(":");
    AnotherLine();
    dir->printNum();
    //递归方法
    for(FAT_Directory* directory:dir->getSubDirectorys()){
        printWithNum(name,directory);
    }
}

//不带数字的打印
void printWithoutNum(string name,FAT_Directory* dir){
    if(name[name.size()-1]!='/'){
        name.append("/");
    }
    name.append(dir->getName());
    printNormal(name);
    printNormal(":");
    AnotherLine();
    dir->print();
    for(FAT_Directory* d:dir->getSubDirectorys()){
        printWithoutNum(name,d);
    }
}

//获得地址vector
vector<string> getPaths(string path){
    vector<string> res;
    if(path.length() == 0) return res;
    char seperator = '/';
    int last_seperator = 0;
    if(path[0]==seperator){
        last_seperator = 1;
    }
    int len = 0;
    for(int i=0;i<path.length();i++){
        if(path.at(i)==seperator){
            if(len==0){
                continue;
            }
            res.push_back(path.substr(last_seperator,len));
            last_seperator = i+1;
            len = 0;
        }
        else{
            len++;
        }
    }
    res.push_back(path.substr(last_seperator));
    return res;
}

//处理ls指令
void executeLS(string args, FAT12& fat12){
    //首先检查命令是否有效
    int isvalid = isValid(args);
    if(isvalid == 0){
        printNormal("wrong parameter!");
        AnotherLine();
        return;
    }
    if(isvalid == 3){
        printNormal("you have input more than one path");
        AnotherLine();
        return;
    }

    bool isLs_l = isvalid==2;
    string path = getPathString(args);
    vector<string> paths = getPaths(std::move(path));
    FAT_Directory* fatDirectory = &fat12.root;

    for (int i = 0; i < paths.size(); i++) {
        //一层层往下寻找
        fatDirectory = fatDirectory->searchDirByName(paths[i]);
        if(fatDirectory == nullptr){
            string error;
            error.append("there is no such dir:").append(paths[i]).append("    please check your path");
            printNormal(error);
            AnotherLine();
            return;
        }
    }
    if (isLs_l){
        printWithNum("/",fatDirectory);
    } else{
        printWithoutNum("/",fatDirectory);
    }

}

int main() {
    FAT12 fat12;
    fat12.setAddress("a.img");
    fat12.load();
    fat12.init();

    printNormal("welcome to fat12");
    AnotherLine();
    printNormal("@lewyWang 王骁");
    AnotherLine();
    printNormal("now please enter your instructions");
    AnotherLine();
    printNormal("> ");
    string instruct;//指令

    string input;
    getline(cin,input);
    for(int i = 0;i<input.length();i++){
        if(input[i] == ' '){
            instruct = input.substr(0,i);
            input = input.substr(i+1);
            break;
        }
    }
    if(instruct.empty()){
        instruct = input;
        input = "";
    }
    //开始循环处理指令

    while (instruct!="exit"||!isAllSpace(input)){
        if (instruct.length()<=1){
            printNormal("your instruction is too short, please input again!");
            AnotherLine();
        } else if(instruct == "ls"){
            executeLS(input,fat12);
        } else if (instruct=="cat"){
            executeCAT(input,fat12);
        } else{
            printNormal("there is no such opcode!please type the right one.");
            AnotherLine();
        }
        //开始下一次循环
        printNormal("> ");
        int index = 0;
        getline(cin,input);
        instruct = "";
        for(;index<input.length();index++){
            if(input[index] == ' '){
                instruct = input.substr(0,index);
                input = input.substr(index+1);
                break;
            }
        }
        if(instruct.empty()){
            instruct = input;
            input = "";
        }
    }

    printNormal("thank you for using, see you");
    AnotherLine();
    return 0;


}

void executeCAT(string basicString, FAT12 fat12) {

}






