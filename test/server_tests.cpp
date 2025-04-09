#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <gtest/gtest.h>


/* These tests aims to test whether server is processing 
commands as expected or not. We will have to run a server instance 
in a separate terminal to run these tests.*/

/*NOTE: Because we are using a seprate instance of server
Make sure to re-start the server for each test, otherwise same 
commands will be run again, and snapshot version will keep increasing*/

class ServerTest : public ::testing::Test {
protected:
        int clientSocket;
        std::string host = "127.0.0.1";
        int port = 8080;

    //We set up the client here. 
    void SetUp() override {
       
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        
        
        // Connect to server
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
        serverAddr.sin_port = htons(port);
        
        int status = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        

    }

    void TearDown() override {
        close(clientSocket);
    }

    void sendCommand (const std::string& command) {
        ssize_t bytesSent = send(clientSocket, command.c_str(), command.length(), 0);
        
    }

    std::string receiveResponse() {
        char buffer[1024];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        // std::cout << "Received response: [" << buffer << "]" << std::endl;
        buffer[bytesRead] = '\0';
        return std::string(buffer);
    }
};

TEST_F(ServerTest, TestInsert) {
    std::string command = "SET tirth great";
    sendCommand(command);
    std::string response = receiveResponse();
    EXPECT_EQ(response, "OK\n");
}

TEST_F(ServerTest, TestGet) {
    std::string command2 = "GET tirth";
    sendCommand(command2);
    std::string response = receiveResponse();
    EXPECT_EQ(response, "OK great\n");
}

TEST_F(ServerTest, TestGetNonExistent) {
    std::string command3 = "GET none";
    sendCommand(command3);
    std::string response = receiveResponse();
    EXPECT_EQ(response, "ERROR Key not found\n");
}

TEST_F(ServerTest, TestSnapshot) {
    std::string command4 = "SET abhigyan supergreat";
    sendCommand(command4);
    std::string response4 = receiveResponse();
    EXPECT_EQ(response4, "OK\n");
    std::string command5 = "SET rijul notgreat";
    sendCommand(command5);
    std::string response5 = receiveResponse();
    EXPECT_EQ(response5, "OK\n");
    std::string command6 = "SNAPSHOT";
    sendCommand(command6);
    std::string response6 = receiveResponse();
    EXPECT_EQ(response6, "OK Snapshot created, version 0\n");
}

TEST_F(ServerTest, TestEditAndCheckRollbackValues) {
    std::string command7 = "EDIT abhigyan supersupergreat";
    sendCommand(command7);
    std::string response7 = receiveResponse();
    EXPECT_EQ(response7, "OK\n");
    std::string command8 = "VGET 0 abhigyan";
    sendCommand(command8);
    std::string response8 = receiveResponse();
    EXPECT_EQ(response8, "OK supergreat\n");
    std::string command9 = "GET abhigyan";
    sendCommand(command9);
    std::string response9 = receiveResponse();
    EXPECT_EQ(response9, "OK supersupergreat\n");
}

TEST_F(ServerTest, TestSnapshotTwice){
    std::string command10 = "SNAPSHOT";
    sendCommand(command10);
    std::string response10 = receiveResponse();
    EXPECT_EQ(response10, "OK Snapshot created, version 1\n");
}

TEST_F(ServerTest, TestDeleteAndCheckRollbackValues){
    std::string command11 = "DEL rijul";
    sendCommand(command11);
    std::string response11 = receiveResponse();
    EXPECT_EQ(response11, "OK\n");
    std::string command12 = "VGET 1 rijul";
    sendCommand(command12);
    std::string response12 = receiveResponse();
    EXPECT_EQ(response12, "OK notgreat\n");
    std::string command13 = "GET rijul";
    sendCommand(command13);
    std::string response13 = receiveResponse();
    EXPECT_EQ(response13, "ERROR Key not found\n");
}


