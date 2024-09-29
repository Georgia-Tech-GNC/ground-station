def cvt_type_to_sql(t: str):
    if t == 'float':
        return 'real'
    elif t == 'int':
        return 'int'
    else:
        raise Exception(f"Invalid type! {t}")
    
def cvt_type_to_str_fmt(t: str):
    if t == 'float':
        return '%.10E'
    elif t == 'int':
        return '%d'
    else:
        raise Exception(f"Invalid type! {t}")

def cvt_type_to_max_len_str(t: str):
    if t == 'float':
        return '1.1234567890E+38'
    elif t == 'int':
        return '-2147483648'
    else:
        raise Exception(f"Invalid type! {t}")