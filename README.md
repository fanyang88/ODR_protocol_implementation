This project I implemented the on-shortest-demand routing algorithm.
I defined a static structure which maintain a routing table, ETH0 IP address, a table mapping all interface mac addresses with each index, the broadcast id, a table keeping record received RREQ, the PF_packet socket descriptor, the unix domain socket descriptor, etc.
I also defined four type of messages. 
1. "UMsg": message received from unix domain socket;
2. "Msg" : message being sent and received in PF_packet frames;
3. "RREP": message contains responsive routing infomation;
4. "RREQ": message used for flooding.
