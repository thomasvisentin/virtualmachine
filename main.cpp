#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <set>
#include <algorithm>
#include <chrono>

std::vector<std::string> split(const char *str, char c = ' ')
{
    std::vector<std::string> result;
    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(std::string(begin, str));
    } while (0 != *str++);

    return result;
}

void Subset(std::vector<std::string> arr, int r,
            int index,  std::vector<std::string> data, int i,
            std::vector<std::set<std::string>> & result){

    if (index == r) {
        std::set<std::string> Sub;
        for (int j = 0; j < r; j++)
            Sub.insert(data[j]);

        if(!Sub.empty())
            result.push_back(Sub);
        return;
    }
    if (i >= arr.size())
        return;
    std::vector<std::string>::iterator it;
    it = data.begin();
    data.insert(index + it, arr[i]);
    Subset(arr, r, index + 1, data, i + 1, result);
    Subset(arr, r, index, data, i + 1, result);
}

int main() {

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();  //TIMER

    std::set <std::string> s;                     //Set of invoice

    std::map <std::string,int> mp;                //Global product

    std::map <std::string,std::set<std::string>> GlobalInvoice; //Invoice global

    float GOAL = 4;
    std::ifstream  data("OnlineRetail_mino.csv");
    std::string line;

    std::getline(data,line);// skip first line

    while(std::getline(data,line)){

        std::vector<std::string> appo = split(&line[0], ';');

        std::string invoiceID = appo.at(0);
        std::string productID = appo.at(1);

        auto inv = GlobalInvoice.find(invoiceID);
        if( inv != GlobalInvoice.end())
             inv->second.insert(productID);
        else{
            std::set<std::string> invo;
            invo.insert(productID);
            GlobalInvoice.insert({invoiceID, invo});
        }

        s.insert(invoiceID);
        mp[productID] += 1;

    }

    data.close();


    //PRINT ALL ELEMENT AND PERCENTAGE
    /* std::cout << "KEY\tELEMENT\n";
    for (auto itr = mp.begin(); itr != mp.end(); ++itr)
        std::cout << itr->first << '\t' << itr->second << "   " << (((float) itr->second) / s.size()) * 100 <<'\n';*/

    int count05 = 0;    int count10 = 0;    int count15 = 0;    int count20 = 0;

    for (auto itr = mp.begin(); itr != mp.end(); ++itr) {

        float percentage = (((float) itr->second) / s.size()) * 100;

        if (percentage > 0.5 && percentage < 1.0)
            count05++;
        else if (percentage > 1.0 && percentage < 1.5 ){
            count10++;  count05++;
        }
        else if (percentage > 1.5 && percentage < 2.0 ){
            count15++;  count10++;  count05++;
        }
        else if (percentage > 2.0){
            count20++;  count15++;  count10++;  count05++;
        }
    }

    std::cout << std::endl << std::endl << "STATISTIC: " << std::endl
    <<"Greater than 0.5%: "<< count05 << std::endl;
    std::cout <<"Greater than 1.0%: "<< count10 << std::endl;
    std::cout <<"Greater than 1.5%: "<< count15 << std::endl;
    std::cout <<"Greater than 2.0%: "<< count20 << std::endl;

    /////////////////////////////////////////////////////////////////
    //FIRST STEP: we remove item that are below the threshold
    ////////////////////////////////////////////////////////////////

    std::cout <<"Size before cutting: "<< mp.size() << std::endl;
    for (auto itr = mp.begin(); itr != mp.end(); ){
         float percentage = (float) itr->second / s.size() * 100;
         if(percentage < GOAL)
             itr = mp.erase(itr);
         else
             ++itr;
    }
    std::cout <<"Size after cutting: "<< mp.size() << std::endl;


    /////////////////////////////////////////////////////////////////
    //SECOND STEP: we create the vector of string to create the subset
    ////////////////////////////////////////////////////////////////
    std::vector<std::string> productset;
    int i = 0;
    for (auto itr = mp.begin(); itr != mp.end(); itr++ )
        productset.push_back(itr->first);


    int r = 2;
    do {
        /////////////////////////////////////////////////////////////////
        //THIRD STEP: we remove elem in the global list that not contain at list one element above threeshold
        ////////////////////////////////////////////////////////////////
        std::cout <<"Erasing elem in the global list that not contain at list one element above threeshold"<< std::endl;

        std::cout <<"GlobalInvoice size: "<< GlobalInvoice.size() << std::endl;

        for(auto itr = GlobalInvoice.begin(); itr!= GlobalInvoice.end(); ){
            int finder = 0;
            for(auto elem : productset )
                if(itr->second.find(elem) != itr->second.end())
                    ++finder;
            if(finder < r)
                itr = GlobalInvoice.erase(itr);
            else
                ++itr;
        }

        std::cout <<"GlobalInvoice size: "<< GlobalInvoice.size() << std::endl;


        ///////////////////////////////////////////////////////////////////////////
        // SUBSET
        ////////////////////////////////////////////////////////////////////////////
        std::vector<std::set<std::string>> result;
        std::vector<std::string> data2;

        Subset(productset, r, 0, data2, 0, result);
        std::cout << "Number of subset of "<< r << " element: " << result.size() << std::endl;

        productset.clear();

        int tenpercent = result.size() * 0.10;        int cont = 0;     //PERCENTAGE

        #pragma omp parallel num_threads(8) firstprivate(GOAL, s, GlobalInvoice)
        {
        #pragma omp for \
        schedule(dynamic) ordered
            for (int i =0; i < result.size(); i++) {
                cont++;
                if (--tenpercent < 0) {
                    tenpercent = result.size() * 0.10;
                    int percentage = (float) cont / (float) result.size() * 100;
                    std::cout << "Cutting progress: " << percentage << " %" << std::endl;
                }

                int total = 0;
                for (auto start: GlobalInvoice)
                    if (std::includes(start.second.begin(), start.second.end(),
                                      result[i].begin(), result[i].end())) {
                        total++;
                    }

                int percentage = (float) total / s.size() * 100;

                if (percentage > GOAL) {
                    std::cout << "Found...";
                    for (auto str: result[i]) {
                        std::cout << " " << str;
                        productset.push_back(str);
                    }
                    std::cout << std::endl;
                }
            }
        }
        //removing duplicates
        sort(productset.begin(), productset.end());
        productset.erase(unique(productset.begin(), productset.end()), productset.end());
        std::cout << "Size after cutting: " << productset.size() << std::endl;
    } while(productset.size()>++r);


    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

    return 0;
}




