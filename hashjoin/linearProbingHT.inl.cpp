class LinearProbingHT {
   public:
   // Entry
   struct Entry {
      uint64_t key;
      uint64_t value;
      std::atomic<bool> marker;
   };

   // Constructor
   LinearProbingHT(uint64_t size) {
   }

   // Destructor
   ~LinearProbingHT() {
   }

   inline Entry* lookup(uint64_t key) {
   }

   inline void insert(uint64_t key) {
   }
};
