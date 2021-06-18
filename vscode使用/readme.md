vscode config里配置以下

Host 192.168.150.123
  HostName 192.168.150.123  // 需要远程连接的主机
  User hongwei.liu
  IdentityFile /Users/user/.ssh/id_rsa   //当前机器的id_rsa路径，然后将
ServerAliveInterval 60000

//将本机的公钥添加到需要远程连接的机器的 authorized_keys里即可免密码登录

//然后 再通过 fn + f1 键  reload windows即可    