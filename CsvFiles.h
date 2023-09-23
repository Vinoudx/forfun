#ifndef _CSVFILES_
#define _CSVFILES_

#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <cstring>

namespace HD{

class CsvFiles
{
    friend bool* rowdeResize(const CsvFiles& arg, bool* rowde);
private:
    size_t total_row;//总行数
    size_t data_row;//读入的数据行数
    size_t m_col;
    std::vector<std::vector<std::string>> m_con;
    std::string m_path; 
    bool* rowde;
    bool* colde;
public:
    

    size_t getTotalRowNum(){return total_row;}
    size_t getDataRowNum(){return data_row;}
    size_t getColNum(){return m_col;}
    

    //增,只能在最后一行或最后一列增加
    //增一个元素
    void addElement(size_t col, const std::string& x);

    //增一行元素
    void addRow(const std::vector<std::string>& x);
    //增加一行空元素
    void addRow();
    //删,一个元素
    void deleteElement(size_t row, size_t col);
    //删，一行元素
    void deleteRow(size_t row){rowde[row - 1] = true;};
    //删，一列元素
    void deleteCol(size_t col){colde[col - 1] = true;};
    //改
    void edit(size_t row, size_t col, const std::string& x);
    //查,一个元素
    std::string getData(size_t row, size_t col){return m_con[row-1][col-1];}
    //查，一行元素
    std::vector<std::string> getRow(size_t row);
    //查，一列元素
    std::vector<std::string> getCol(size_t row);
    //保存
    void save();
    void save(std::string path);
    CsvFiles():total_row(0),data_row(0),m_col(0){};

    //带地址的构造函数，begin为标题行的数量
    CsvFiles(const std::string& path, size_t begin = 0);
    CsvFiles(const CsvFiles& x) = delete;
    CsvFiles& operator=(const CsvFiles& x) = delete;

    ~CsvFiles(){delete []rowde;delete []colde;};

};

CsvFiles::CsvFiles(const std::string& path, size_t begin){
    m_path = path;
    std::ifstream ifs;
    ifs.open(path,std::ios::in);
    if(!ifs.is_open()){ifs.close();throw std::runtime_error("open fail");}
    std::string buf;
    std::string temp;
    data_row = 0;
    total_row = 0;
    size_t col = 0;
    size_t len = 0;
    m_col = 0;
    while(std::getline(ifs,buf)){
        ++total_row;
        if(total_row <= begin)continue;
        m_con.emplace_back(std::vector<std::string>());
        len = buf.size();
        int i = 0;
        for(i = 0 ; i < len ; i++){
            if(buf[i] != ','){
                temp.push_back(buf[i]);
            }else{
                if(i != 0){
                    if(buf[i-1] == ','){
                        m_con[data_row].emplace_back("\0");
                    }else{
                        m_con[data_row].emplace_back(temp);
                    }
                }else{
                    m_con[data_row].emplace_back("\0");
                }
                temp.clear();
                ++col;
            }
        }
        if(buf[i-1] == ','){
            m_con[data_row].emplace_back("\0");
        }else{
            m_con[data_row].emplace_back(temp);
        }
        //m_col = std::max<int>(m_col,col+1);
        m_col = m_col == 0 ? col + 1 : m_col;
        col = 0;
        ++data_row;
        buf.clear();
        temp.clear();
    }
    ifs.close();
    
    rowde = new bool [data_row] ();
    colde = new bool [m_col] ();
}

void CsvFiles::edit(size_t row, size_t col,const std::string& x){
    if(row > data_row){throw std::out_of_range("row out of range");}
    if(col > m_col){throw std::out_of_range("col out of range");}
    m_con[row-1][col-1] = x;
}


void CsvFiles::addElement(size_t col,const std::string& x){
    if(col > m_col){throw std::out_of_range("col out of range");}
    
    m_con.emplace_back(std::vector<std::string>());
    for(int i = 0 ; i < col - 1 ; i++){
        m_con[data_row].emplace_back("\0");
    }
    m_con[data_row].emplace_back(x);
    for(int i = col ; i < m_col ; i++){
        m_con[data_row].emplace_back("\0");
    }
    ++data_row;
    ++total_row;
    rowde = rowdeResize(*this,rowde);
}


inline void CsvFiles::deleteElement(size_t row, size_t col){
    m_con[row-1][col-1] = "\0";
}

void CsvFiles::addRow(const std::vector<std::string>& x){
    if(x.size() > m_col){throw std::out_of_range("col out of range");}
    m_con.emplace_back(x);
    ++data_row;
    ++total_row;
    rowde = rowdeResize(*this,rowde);
}

void CsvFiles::addRow(){
    std::vector<std::string> x;
    x.resize(m_col);
    addRow(x);
    rowde = rowdeResize(*this,rowde);
}

void CsvFiles::save(){
    save(m_path);
}

void CsvFiles::save(std::string path){
    std::ofstream ofs;
    ofs.open(path,std::ios::out | std::ios::trunc);
    if(!ofs.is_open()){ofs.close();throw std::runtime_error("open fail");}
    for(int i = 0 ; i < data_row ; i++){
        if(!rowde[i])continue;
        for(int j = 0 ; j < m_col ; j++){
            if(!colde[j])continue;
            ofs<<m_con[i][j];
            if(j != m_col - 1)ofs<<',';
        }
        ofs<<'\n';
    }
    ofs.close();
}

std::vector<std::string> CsvFiles::getRow(size_t row){
    if(row > data_row){throw std::out_of_range("row out of range");}
    std::vector<std::string> res;
    for(int i = 0 ; i < m_col ; i++){
        res.emplace_back(m_con[row - 1][m_col]);
    }
    return res;
}

std::vector<std::string> CsvFiles::getCol(size_t col){
    if(col > m_col){throw std::out_of_range("col out of range");}
    std::vector<std::string> res;
    for(int i = 0 ; i < data_row ; i++){
        res.emplace_back(m_con[i][col - 1]);
    }
    return res;
}

bool* rowdeResize(const CsvFiles& arg, bool* rowde){
    bool* temp = new bool[arg.data_row + 1] ();
    memcpy(temp,rowde,arg.data_row);
    return temp;
}

}

#endif