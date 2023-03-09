# Project 4: Implementation of Dictionary Codec

Multithreading for data insertion/encoding:

Data from input will be segmented, encoded into separate hash tables, and then those separate hash tables will be merged.

Going to use a hash table encoding method and use standard library unordered_map to for the hash table.
Output encoding will be put in a standard library vector.

Need to decide if we should take in all column data at once and then breakup.
Probably should since we need to do a vanilla column search/scan also in the project.
