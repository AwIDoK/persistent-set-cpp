#include "../persistent_set.h"
#include "iostream"

using namespace std;

int main() {
    persistent_set<int> st;
    st.insert(1);
    st.insert(2);
    return 0;
   /* for (int i = 0; i < 5; i++) {
        st.insert(i);
    }
    for (int i = 0; i < 5; i++) {
        st.insert(i);
    }
   // auto f = st.find(4);
    st.erase(st.find(2));
   /* for (int i = -1; i < 6; i++) {
        cout << (st.find(i) != st.end());
    }
    cout << endl;*/
    for (auto it : st) {
        cout << it << ' ';
    }
}
