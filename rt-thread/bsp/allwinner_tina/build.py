#!/usr/bin/python
# -*- coding:utf-8 -*-
from __future__ import print_function
PYTHONUNBUFFERED=1
ElfFullName="c:/Users/DELL/Desktop/rt-thread/bsp/allwinner_tina/rtthread.elf"
HexFullFile="c:/Users/DELL/Desktop/rt-thread/bsp/allwinner_tina/rtthread.hex"
HideDownloadInfo=True
HideBuildInfo=True
HideCleanInfo=True
Compare=True
GernateHex=True
CalUsage=True
Build=True
BuildCost=True
Download=False
Device="STM32F407IG"
FLASH_SIZE=1024
MEM_SIZE=192
JLINKDIR='D:/BuildTools/SEGGER/JLink/JLink.exe'
Toolchain="arm-none-eabi-gcc"
ToolchainSize="arm-none-eabi-size"
ToolchainObj="arm-none-eabi-objcopy"
#:&1& do not change this two line #################################################
#:&1& ShareFun 自定义函数相关 ######################################################
Clean=False

# 用于检测到命令执行错误，直接退出 打断后续输出
NeedExit=False

import sys
import os
import math
import datetime
import time
import timeit
import subprocess
import re
buildResultFile='.vscode/buildResult.txt'

#配置命令参数
import getopt
import sys

opts,args = getopt.getopt(sys.argv[1:],'-d:-e:-f:-n:-c-g-h:-k-f:-v-l-r',
['device=','elfFile=','flashsize=','memsize=','compare','gernatehex','clean','downloadOnly','rebuild'])
# print(opts)
for opt_name,opt_value in opts:
    if opt_name in ('-d','--device'):
        Device=opt_value
    if opt_name in ('-m','--memsize'):
        MEM_SIZE=opt_value
    if opt_name in ('-f','--flashsize'):
        FLASH_SIZE=opt_value
    if opt_name in ('-e','--elfFile'):
        ElfFullName=opt_value
    if opt_name in ('-c','--compare'):
        Compare=True
    if opt_name in ('-g','--gernatehex'):
        GernateHex=True
    if opt_name in ('-h','--HexFullFile'):
        HexFullFile=opt_value
    if opt_name in ('-l','--downloadOnly'):
        Compare=False
        GernateHex=False
        Build=False
        BuildCost=False
        Download=False
        CalUsage=False
        Download=True
        Clean=False
    if opt_name in ('-k','--clean'):
        Compare=False
        Build=False
        GernateHex=False
        BuildCost=False
        Download=False
        CalUsage=False
        Download=False
        Clean=True
    if opt_name in ('-r', '--rebuild'):
        Compare = False
        Build = True
        GernateHex = False
        BuildCost = False
        Download = False
        CalUsage = False
        Download = False
        Clean = True
# 智能转换数值为相应容量
def ConvertToDecimalPoint(bytes, lst=['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB','EB', 'ZB' ,'YB', 'BB']):
    if bytes == 0:
        return '0  Byte'
    i = int(math.floor( # 舍弃小数点，取小
             math.log(abs(bytes), 1024) # 求对数(对数：若 a**b = N 则 b 叫做以 a 为底 N 的对数)
            ))

    if i >= len(lst):
        i = len(lst) - 1
    return ('%.2f' + " " + lst[i]) % (bytes/math.pow(1024, i))

import linecache


def get_line_context(file_path, line_number):
    return linecache.getline(file_path, line_number).strip()

def targetName():
    filepath, tmpfilename = os.path.split(ElfFullName)
    shotname, extension = os.path.splitext(tmpfilename)
    return shotname


def printWithColor(strs):
    print('\033[1;33m'+strs+'\033[0m')


def printWithRed(strs):
    print('\033[1;31m'+strs+'\033[0m')

def printWithBlue(strs):
    print('\033[1;34m'+strs+'\033[0m')

def getBlueColorStr(strs):
    return '\033[1;34m'+strs+'\033[0m'

def getRedColorStr(strs):
    return "\033[1;31m"+strs+"\033[0m"

# 输出字符串 尤其是同行输出功能开始的时候
def printString(printStr,stste,normalMode=True,strBefore='',colorMode=''):
    if not HideCleanInfo:
        normalMode=True

    printStr2 = printStr.replace('\n','').replace('\r','')
    if(normalMode):
        # 前一行不是正常输出行
        printStr2=printStr2
        if not stste['preLineIsNormal']:
            printStr2=os.linesep+printStr2
            stste['preLineIsNormal']=True
        if(colorMode=='Red'):
            printStr2=getRedColorStr(printStr2)
        print(printStr2, end=os.linesep)
    else:
        prtstr=str(stste['filesNum'])+" "+printStr2
        prtstr=prtstr.ljust(stste['lastLineLength'])
        print( '\r' + getBlueColorStr(strBefore+prtstr), end=" ")
        stste['lastLineLength']=len(prtstr)
        stste['preLineIsNormal']=False

    # sys.stdout.flush()


def SimplifyPrint(nextline,state):
    unknowStr=True
    # 异常信息优先输出
    if nextline.upper().find('CANNOT')!=-1:
        printWithRed(nextline)
        unknowStr = False
    if nextline.find('FAILED')!=-1:
        printWithRed(nextline)
        unknowStr = False

    # 需要隐藏的部分
    objFile=r'(.*(\.o)$)|(^Removed .*)'
    objFileMatch = re.search(objFile, nextline, re.IGNORECASE)
    if objFileMatch:
        state['filesNum']+=1
        printString(nextline,state,False,"Num:")
        unknowStr = False

    errStr = r'.*(warning|error)(\s+|:).*'
    errStrMatch = re.search(errStr, nextline, re.IGNORECASE)
    if errStrMatch:
        printString(nextline, state, True, "Num:",'Red')
        unknowStr = False

    # 输出未匹配的内容
    if(unknowStr):
        printString(nextline,state,True,"")



def clean(cleanCmd):
    if(Clean):
        printWithColor(cleanCmd)
        if not HideCleanInfo:
            os.system(cleanCmd)
        else:
            s=subprocess.Popen(cleanCmd,bufsize=0,stdout=subprocess.PIPE,universal_newlines=True,shell=True)
            state={'filesNum':0,'lastLineLength':0,'preLineIsNormal':True}
            count=0
            while True:
                nextline=s.stdout.readline()
                if(count>=5):
                    break
                if nextline.strip() == "":
                    count+=1
                    continue
                else:
                    count = 0

                # 输出结果
                SimplifyPrint(nextline,state)
            time.sleep(0.1)
            printString("total files: "+str(state['filesNum']),state,True,"")


def build(buildCmd):
    global ElfFullName
    global HexFullFile
    global NeedExit
    if(Build):
        printWithColor(buildCmd)
        if not HideBuildInfo:
            os.system(buildCmd)
        else:
            state = {'filesNum': 0, 'lastLineLength': 0,
                     'preLineIsNormal': True}
            count=0
            res=subprocess.Popen(buildCmd,bufsize=0,stdout=subprocess.PIPE,universal_newlines=True,shell=True)
            # res=subprocess.Popen(buildCmd,bufsize=0,stdout=subprocess.PIPE,stderr=subprocess.PIPE,universal_newlines=True,shell=True)
            while True:
                # err = res.stderr.read()
                # if err:
                #     printWithRed(err)
                #     NeedExit=True
                # if res.poll()!=None:
                #     print("主动结束")
                #     break
                nextline = res.stdout.readline()
                if(count>=5):
                    break
                if nextline.strip() == "":
                    count+=1
                    # print("empty")
                    continue
                else:
                    count = 0

                # 如果elf或者hex文件之前不存在，此处智能检测补充
                if ElfFullName.strip()=='':
                    elfFile=r'.*(\.elf)$'
                    elfFileMatch = re.search(elfFile, nextline, re.IGNORECASE)
                    if elfFileMatch:
                        elf=re.findall(r'[^\s]*\.elf',nextline)
                        if(len(elf)>0):
                            ElfFullName = elf[len(elf)-1]
                if ElfFullName.strip()!='' and HexFullFile.strip()=='':
                    HexFullFile=re.sub(r'\.elf$','.hex',ElfFullName)

                # 输出结果
                SimplifyPrint(nextline,state)
            time.sleep(0.1)
            printString("total files: "+str(state['filesNum']),state,True,"")

if(BuildCost):
    # start = time.clock()
    start = timeit.default_timer()



#:&2& do not change this two line #################################################
#:&2& start 编译前记录编译结果 ######################################################
Flash_Before=0
MEM_Before=0
if(Compare):
    #存在编译结果记录 则直接读取
    if os.path.exists(buildResultFile):
        rbefore = get_line_context(buildResultFile,2)
        # print (rbefore)
        arrbefore=rbefore.split("\t")
        Flash_Before = int(arrbefore[0]) + int(arrbefore[1])
        MEM_Before = int(arrbefore[1]) + int(arrbefore[2])
    else:#储存编译结果不存在
        if os.path.exists(ElfFullName):
            resultbefore = os.popen('arm-none-eabi-size '+ ElfFullName)
            time.sleep(0.1)
            arrbefore = resultbefore.read().splitlines()[1].split("\t")
            Flash_Before = int(arrbefore[0]) + int(arrbefore[1])
            MEM_Before = int(arrbefore[1]) + int(arrbefore[2])

if(Clean):
    Clean=True
    # print('loading clean task')


if(Build):
    Build = True
    # print('loading build task')


clean("scons -c")
build("scons")
#:&3& do not change this two line ##################################################
#:&3& startGenerateHexFun  #########################################################
if(NeedExit):
    sys.exit()

if(GernateHex):
    printWithColor("Gernate Hex File")
    if not os.path.exists(ElfFullName):
        printWithRed("ELF File missing Can not gernate Hex File")
    else:
        print('from ' + ElfFullName)
        gernateCmd = ToolchainObj +" -O ihex "+ElfFullName+" "+targetName() +".hex"
        if os.system(gernateCmd)  !=0:
            printWithColor("Gernate Hex File Error")
        else:
            printWithColor("Gernate Hex File Success " +targetName() +".hex")


#:&4& do not change this two line ##################################################
#:&4& start 程序占比计算 ############################################################
if(CalUsage or Compare):
    if not os.path.exists(ElfFullName):
        printWithRed("Before Build No ELF File Try again")
        CalUsage=False
        Compare=False
    else:
        result = os.popen('arm-none-eabi-size ' +ElfFullName)
        r = result.read()
        time.sleep(0.1)
        # print (r)
        arr=r.splitlines()[1].split("\t")
        flash = int(arr[0]) + int(arr[1])
        mem = int(arr[1]) + int(arr[2])
        flash_size=FLASH_SIZE*1024
        mem_size=MEM_SIZE*1024
        flash_usage = float(flash*100)/flash_size
        mem_usage = float(mem*100)/mem_size
        # print ("")
        if(CalUsage):
            print ("-------------------------------------------------------")
            print ('Flash:  %8s / %8s , %4.2f%% (.text + .data)'%(ConvertToDecimalPoint(flash),ConvertToDecimalPoint(flash_size),flash_usage))
            print ('SRAM:   %8s / %8s, %4.2f%% (.data + .bss )'%(ConvertToDecimalPoint(mem),ConvertToDecimalPoint(mem_size),mem_usage))

        writestr=""
        writestr=r.splitlines()[0] + '\n'
        writestr+=r.splitlines()[1]+ '\n'
        writestr+="BuildTime:%s\n"%datetime.datetime.now()
        writestr+="-------------------------------------------------------\n"

        if os.path.exists(buildResultFile):
            with open(buildResultFile,mode='r+') as f:
                content = f.read()
                f.seek(0, 0)
                f.write( writestr + content)
        else:
            if not os.path.exists('.vscode'):
                os.makedirs('.vscode')
            with open(buildResultFile, mode='w') as ff:
                ff.write( writestr)



#:&5& do not change this two line ##################################################
#:&5& start 编译前比对编译结果 #######################################################
if(CalUsage):
    flash_change=flash-Flash_Before
    mem_change=mem-MEM_Before
    flash_usageChange=float(flash_change*100)/flash_size
    mem_usageChange=float(mem_change*100)/flash_size
    print ("-------------------------------------------------------")
    if(flash_usageChange==0 and flash_usageChange==0):
        printWithColor("No Change")
    else:
        if(flash_usageChange!=0):
            printWithColor ('Flash Change:  %8s / %8s , %4.3f%%'%(ConvertToDecimalPoint(flash_change),ConvertToDecimalPoint(flash_size),flash_usageChange))
        if(mem_usageChange!=0):
            printWithColor ('SRAM  Change:  %8s / %8s, %4.3f%% '%(ConvertToDecimalPoint(mem_change),ConvertToDecimalPoint(mem_size),mem_usageChange))



#:&6& do not change this two line ##################################################
#:&6& start 收尾工作相关 ############################################################
# end = time.clock()
if(BuildCost):
    end = timeit.default_timer()
    time_local = time.localtime(end-start)
    dt = time.strftime("Build Time %M:%S",time_local)
    printWithColor (dt)



#:&7& do not change this two line ##################################################
#:&7& start 下载程序的相关功能 #######################################################
# 选择性显示输出的内容，高亮重点内容
def SelectiveOutputJlinkInfo(nextline):
    # print(nextline)
    unknowStr=True
    if nextline.find("Script processing completed.")==0:
        return False
    if nextline.strip() == "":
        return True

    # 需要着重显示

    if nextline.find('Cannot')!=-1:
        printWithRed(nextline)
        unknowStr = False
        return True
    if nextline.find('Failed')!=-1:
        printWithRed(nextline)
        unknowStr = False
        return True
    if nextline.find('Downloading file')==0:
        printWithColor(nextline)
        unknowStr = False
        return True
    if nextline.find('Contents already match')==0:
        printWithColor(nextline)
        unknowStr = False
        return True
    sucessDown=r'^(J-Link: Flash download: Bank).*'
    matchObj2 = re.search(sucessDown, nextline)
    if matchObj2:
        printWithBlue(nextline)
        unknowStr = False
        return True

    #隐藏部分无用输出
    if(HideDownloadInfo):
        # 前面未曾过滤掉的
        if unknowStr:
            reg=r'^(SEGGER J-Link Commander).*$'
            reg+=r'|^Hardware version.*$'
            reg+=r'|^CPUID register.*$'
            reg+=r'|^Firmware:.*$'
            reg+=r'|^DLL version.*$'
            reg+=r'|^S/N.*$'
            reg+=r'|^Iterating through.*$'
            reg+=r'|(Connecting to target via).*$'
            reg+=r'|^Processing script file.*$'
            reg+=r'|^Target connection.*$'
            reg+=r'|^Device ".*selected.*$'
            reg+=r'|^J-Link connection.*$'
            reg+=r'|^License\(s\).*$'
            reg+=r'|.*identified.$'
            reg+=r'|^CoreSight components.*$'
            reg+=r'|^Scanning.*$'
            reg+=r'|^Found .*$'
            reg+=r'|^Script processing completed.*$'
            reg+=r'|(FPS)+|^R[0-9].=.|^PC.=.|^Reset.*$|^ROMTbl.*$'
            reg+=r'|^DPIDR.*$|^AP.*$|^FPUnit.*$|^SP\(R13\).*$|^XPSR =.*$|^CFBP =.*$'
            reg+=r'|^(\s*)\r\n'
            matchObj = re.search(reg, nextline)
            if not matchObj:
                print(nextline)
            # else:
            #     print("---"+nextline)
    else:
        print(nextline)

if(Download):
    cmdstr= JLINKDIR+' -device '+ Device +' -CommandFile '+CommandFileName
    s=subprocess.Popen(cmdstr,bufsize=0,stdout=subprocess.PIPE,universal_newlines=True,shell=True)
    count =0
    while True:
        nextline=s.stdout.readline()
        ret = SelectiveOutputJlinkInfo(nextline.strip())
        # print(nextline.strip())
        if(ret==False):
            break
        if nextline.strip() == "":
            count+=1
        else:
            count = 0
        if(count==5):
            break
    time.sleep(0.1)


if(os.path.exists('c:/Users/DELL/Desktop/rt-thread/bsp/allwinner_tina/build.py')):os.remove('c:/Users/DELL/Desktop/rt-thread/bsp/allwinner_tina/build.py')
