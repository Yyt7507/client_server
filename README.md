# client_server
使用C++、Qt设计的一套基于C/S架构的局域网即时通讯系统，包含独立的服务端与客户端

## 功能
- 注册
- 登陆
- 好友列表
- 一对一聊天
- 上线、离线消息提醒
- 未读消息提醒
- 历史聊天记录保存

## 编译
Qt

## 提醒
server端可以直接用Qt运行
client端需要把client文件夹下的ipconfig.ini文件拷贝到build-client-Desktop_Qt_5_12_0_MinGW_64_bit-Debug/Debug/中，然后根据实际情况修改ipconfig.ini中serverip的值(这样设计是为了应对client端生成.exe打包文件，能方便直接运行)

## 作者
Yyt7507
