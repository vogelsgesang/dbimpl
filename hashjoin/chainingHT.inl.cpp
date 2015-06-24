class ChainingHT {
   public:
   // Chained tuple entry
   struct Entry {
      uint64_t key;
      uint64_t value;
      Entry* next;
   };

   // Constructor
   ChainingHT(uint64_t size) {
   }

   // Destructor
   ~ChainingHT() {
   }

   inline Entry* lookup(uint64_t key) {
   }

   inline void insert(Entry* entry) {
   }
};
