#ifndef MODEL_HPP
#define MODEL_HPP

#include "header.hpp"
#include "const.hpp"

struct command {
    vector<string> args;
    pipe_type pipe;
    int pipeNum;
    bool fileRedirect;
    string outputFile;

    command() : pipe(PIPE_NONE), pipeNum(0) {}

    void addArg(string arg) {
        this->args.push_back(arg);
    }

    void setPipe(pipe_type pipe, int pipeNum) {
        this->pipe = pipe;
        this->pipeNum = pipeNum;
    }

    void setFileRedirect(string outputFile) {
        this->fileRedirect = true;
        this->outputFile = outputFile;
    }

    bool isPipe() {
        return this->pipe != PIPE_NONE;
    }

    bool isPipeNormal() {
        return this->pipe == PIPE_NORMAL;
    }

    bool isPipeNumbered() {
        return this->pipe == PIPE_NUMBERED;
    }

    bool isPipeNumberedErr() {
        return this->pipe == PIPE_NUMBERED_ERR;
    }

    bool isFileRedirect() {
        return this->fileRedirect;
    }

    void print() {
        for (auto arg : this->args) {
            cout << arg << " ";
        }
        cout << "\n";
        cout << "pipe: " << this->pipe << "\n";
        cout << "pipeNum: " << this->pipeNum << "\n";
        cout << "fileRedirect: " << this->fileRedirect << "\n";
        cout << "outputFile: " << this->outputFile << "\n";
    }
};

struct numberedPipe {
    int readFd;
    int writeFd;
    int count;

    numberedPipe(int readFd, int writeFd, int count) : readFd(readFd), writeFd(writeFd), count(count) {}
};

struct numberedPipes {
    vector<numberedPipe> nbpps;

    numberedPipes() {}
    ~numberedPipes() {
        for (auto& pipe : nbpps) {
            close(pipe.readFd);
            close(pipe.writeFd);
        }
    }

    void print() {
        cout << "length: " << nbpps.size() << "\n";
        for (auto pipe : nbpps) {
            cout << "pipe: " << pipe.readFd << " " << pipe.writeFd << " count: " << pipe.count << "\n";
            cout << "--------------------------------\n";
        }
    }

    void addPipe(numberedPipe pipe) {
        nbpps.push_back(pipe);
    }

    void increaseCount() {
        for (auto& pipe : nbpps) {
            pipe.count++;
        }
    }

    void decreaseCount() {
        for (auto& pipe : nbpps) {
            pipe.count--;
        }
    }

    void resetCountOneToZero() {
        for (auto& pipe : nbpps) {
            if (pipe.count == 1) {
                pipe.count = 0;
            }
        }
    }

    void releaseInvalidPipes() {
        nbpps.erase(
            remove_if(nbpps.begin(), nbpps.end(), 
                [](const numberedPipe& p) { 
                    if (p.count < 0) {
                        close(p.readFd);
                        close(p.writeFd);
                    }
                return p.count < 0;
            }),
            nbpps.end()
        );
    }

    int getPipeWithCountIsZero() {
        for (int i = 0; i < nbpps.size(); i++) {
            if (nbpps[i].count == 0) {
                return i;
            }
        }
        return NOT_FOUND;
    }

    int getOutputPipeWithSameCount(int count) {
        for (auto pipe : nbpps) {
            if (pipe.count == count) {
                return pipe.writeFd;
            }
        }
        return NOT_FOUND;
    }

    numberedPipe getPipeWithIndex(int index) {
        return nbpps[index];
    }
};

#endif