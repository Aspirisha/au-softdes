##I2I

**I2I** is a simple messenger written for software design course. It supports two basic protocols: I2I protocol (native) and [TinyChat 9000 messenger protocol]( https://github.com/edgarzhavoronkov/au-software-design-fall-2016/tree/messenger)

##Build
1. git clone --recursive https://github.com/Aspirisha/au-softdes.git
2. cd au-softdes
3. git checkout task8
4. cd I2I
5. mkdir build && cd build
6. qmake ..
7. make -j8

##Run
After build cd to i2i/ and run i2i. You will be asked to choose server ip and port (this will be used to connect to your client by another clients). If the port is available for listening, the client window will show up.

When new I2I users arrive (not **Tiny9000** users), they are shown in the window "peers" on the left. You can send message to any of them selecting the desired recipient and writing message in the text field in the bottom of the window. 

To connect to **Tiny9000** client, type his or her ip and port in the left input fields and click connect. Note that connecting in such a way to **I2I** client is not needed.
