#!/usr/bin/env python3

def output_head(out):
    out.write('// jawc dictionary data\n')
    out.write('\n')
    
def output_header_array(name, out):
    out.write('extern const char *%s[];\n' % name)
    out.write('extern const unsigned %s_size;\n' % name)

def output_data_array(name, data, out):
    out.write('const char *%s[] = {\n' %name)
    
    for word in data:
        word = word.replace('\\', '\\\\').replace('"', '\\"').replace('?', '\\?')
        out.write('   "%s",\n' % word)
    out.write('};')
    out.write('\n')
    out.write('const unsigned %s_size = %d;\n' % (name, len(data)))

with open('answers.txt', 'r') as answers_txt, open('allowed-guesses.txt', 'r') as words_txt:
    
    answers = [word.rstrip() for word in answers_txt]
    words = [word.rstrip() for word in words_txt]
    
with open('src/dict.c', 'w') as out_c, open('src/dict.h', 'w') as out_h:
    output_head(out_h)
    
    out_h.write('\n')
    out_h.write('#ifndef _JAWC_DICT_H_\n')
    out_h.write('#define _JAWC_DICT_H_\n')
    out_h.write('\n')
    
    output_header_array('answers', out_h)
    output_header_array('words', out_h)
    out_h.write('#endif\n')
    

    output_head(out_c)
    output_data_array('answers', answers, out_c)
    out_c.write('\n')
    output_data_array('words', words, out_c)
    out_c.write('\n')

    
    