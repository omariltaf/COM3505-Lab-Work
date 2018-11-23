# build-word-tables.py

# preliminaries
import pprint, sys
testing = False # True # set false for the real thing...

# map from chars to input symbol numbers
char_gps = [' ', 'abc', 'def', 'ghi', 'jkl', 'mno', 'pqrs', 'tuv', 'wxyz']
char_map = {}
for i in range(len(char_gps)):
    for c in char_gps[i]:
        char_map[c] = str(i + 1)

# map input num sequence to top candidate words
# candidates = any complete words having this numeric sequence, plus
#   most-frequent words with this sequence as a prefix
candidates = {}
maxcandidates = 10
suggestionSets = {}
# TODO make this parameterisable
maxlexiconsize = 100

# add words into main store
def add_to_store(wd):
    nums = ''.join(char_map[c] for c in wd) # compute num string for wd
    for i in range(len(nums)):
        prefix = nums[:i+1]
        if prefix in candidates:
            cands = candidates[prefix]
        else:
            candidates[prefix] = cands = []
        if i+1 == len(wd): 
            cands[0:0] = [wd] # ensure full word is a candidate (add to front)
        else:
            cands.append(wd)
        cands[maxcandidates:] = [] # trim down to max len

# lookup function
def get_candidate(nums):
    if nums in candidates:
        return candidates[nums]
    else:
        return []

# basic test UI
def basic_test():
   while 1:
       nums = input('=> ')
       cands = get_candidate(nums)
       for cand in cands:
           print(cand)

# load up the word list and populate main store, either locally or from file
words = [ 'the', 'and', 'there', 'than' ] # for testing
words = [ 'the', 'egg', 'there', 'than', 'a', 'pig' ] # for testing
if not testing: # load words from file (must be sorted by frequency)
  words = []
  c = 0
  wordFile = 'words-by-freq.txt'
  if len(sys.argv) > 1:
      wordFile = sys.argv[1]
  with open(wordFile) as infs:
      for line in infs:
          wd = line.strip()
          words.append(wd)
          c += 1
          if c >= maxlexiconsize:
              break

# create an array model of the word list and print it in C++
wordNumMap = dict() # maps each word to its array position
print('// predictor.h\n')
print('#ifndef PREDICTOR_H')
print('#define PREDICTOR_H')
print('static const uint16_t NUM_WORDS = ', len(words), ';', sep='')
print('''
class Predictor {
  static const uint16_t MAX_WORD_LEN = 20; // max characters in a word
  uint16_t STATE_SUGGESTIONS = 0; // position of sugset num in state array
  uint16_t STATE_NUM_DESCS = 1;   // position of sugset num in state array
  uint16_t STATE_1ST_DESC = 2;    // position of 1st descendant in state array

  uint16_t state = 0;         // array index of current state (0 is root)
  char history[MAX_WORD_LEN]; // symbols consumed so far
  uint16_t histlen = 0;       // number of symbols in history
  uint16_t sugiter = 1;       // array index for suggestion set iterator
public:
  Predictor();
  void print();
  void reset();
  int16_t suggest(uint8_t symbolSeen);
  const char *next();
  const char *first();
  uint16_t getState();
};
''')
print('static const char *words[] = {  // in order of frequency')
i = 0
for w in words:
    add_to_store(w)
    print(
      "".join([ '  "', w, '",', '//'.rjust(20 - len(w)) ]),
      "%4d" % i, end=''
    )
    print()
    wordNumMap[w] = i
    i = i + 1
print('};\n')

# copy candidates to map with suggestion tuples as keys
tcands = dict()
for item in candidates.items():
    tcands[item[0]] = tuple(item[1])

# create reverse mapping of suggestion tuples to number sequence sets
suggs2seqs = {}
for k, v in tcands.items():
    suggs2seqs[v] = suggs2seqs.get(v, [])
    suggs2seqs[v].append(k)

# print C++ table of suggestion sets (and map from sugg set num to set)
suggs2num = dict()
print('static const uint16_t NUM_SUGGS = ', len(suggs2seqs), ';', sep='')
print('static const uint16_t suggestionSets[][NUM_SUGGS] = {')
print('//arity wordnum(s)...                     index: symseq(s)...')
i = 0
for item in sorted(suggs2seqs.items()):
    suggset = item[0]
    symseqs = item[1]
    print('  { ', end='')
    print("%2d,  " % len(suggset), sep='', end='')
    wlist = list()
    listlen = 0
    for w in suggset:
        wnum = wordNumMap[w]
        print(wnum, ", ", sep='', end='')
        listlen += len(str(wnum)) + 2
        wlist.append(wnum)
    print(
      '}, ', '//'.rjust(30 - listlen), "%5d" % i,
      ': ', " ".join(sorted(symseqs)), sep=''
    )
    suggs2num[tuple(wlist)] = i
    i = i + 1
print('};')

# make a mapping from suggestion tuples to num seqs using word numbers
seqs2nums = {}
for k, v in suggs2seqs.items():
    nums = list()
    for w in list(k):
        nums.append(wordNumMap[w])
    seqs2nums[tuple(nums)] = v

# print some comments for debugging purposes
print('\n/*')
#print('candidates:\n', candidates)
print('candidates:')
pprint.pprint([[s, candidates[s]] for s in sorted(candidates)])
#print('tcands:\n',     tcands)
#print('suggs2seqs:\n', suggs2seqs)
#print('seqs2nums:\n',  seqs2nums)
#print('suggs2num:\n',  suggs2num)

# input symbol sequences mapped to suggestion set array indices
print('\ninput sym sequences to suggestion sets:')
for numseq in sorted(candidates):
     suggs = [wordNumMap[w] for w in candidates[numseq]]
     suggTuple = tuple(suggs)
     print(numseq, '	', suggs2num[suggTuple])

# build state transition table and print to C++
# states have: descendants (up to NUM_SYMS), a suggestion set
# each continuous column in the ordered sym sequences is a state
suggsnum = -1                   # suggestion set number (for root = -1)
stnum = 0                       # state number (= array index)
root = [stnum, [], suggsnum]    # state num, path, suggestion set
states = [ root ]               # the transition table, each row is a state
precursors = [ root ]           # the precursors to the current state
prevdepth = -1                  # the depth of the previous state (root = -1)

for path in sorted(candidates): # for each path/sugg set
     depth = len(path) - 1      # depth of new state
     delta = depth - prevdepth  # different between new depth and previos
     if delta <= 0:             # we've gone back up the tree, so...
         del precursors[:abs(delta) + 1] # ...move back up precursors
     parent = precursors[0]     # where to attach the new state
     suggsnum = \
         suggs2num[tuple([wordNumMap[w] for w in candidates[path]])] # sset #
     symbol = path[depth]       # the symbol that the new state consumes
     stnum = stnum + 1          # state number (= array index)
     nextstate = [ stnum, [], suggsnum ]    # create the new state
     parent[1].append([int(symbol), stnum]) # attach the new state to parent
     precursors.insert(0, nextstate)        # prepend new state to precursors
     states.append(nextstate)   # remember the state in the transition table
     prevdepth = depth          # remember current depth

# debug print
print('\nstates:')
for s in states: print(s)
print('*/\n')

# print to C++
print('static const uint16_t NUM_STATES = ', len(states), ';', sep='') # states could..
print('static const int16_t states[][NUM_STATES] = {') # ...be uint if root sugs not -1
i = 0
for s in states:
    if not i % 10: 
      print('// sugset numds desc(s)[sym/state]....                             ... // index')
    i = i + 1
    print('  { ', "%5d, " % s[2], "%2d, " % len(s[1]), sep='', end='')
    listlen = 0
    for desc in s[1]:
        listlen += 10
        print("%2d, " % desc[0], sep='', end='')
        print("%4d" % desc[1], ', ', sep='', end='')
    print('}, ', sep='', end='')
    print(('// %4d' % s[0]).rjust(60 - listlen), sep='')
print('};')
print('#endif')
