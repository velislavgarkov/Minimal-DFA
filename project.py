import subprocess

def SymbolSet(chars):
    literals = list(chars)
    return "(" + "|".join(literals) + ")"

log = open("log.txt", "w")

def stat(expr):
    result = subprocess.run(["./project"], input=expr.encode("utf-8"), capture_output=True)
    log.write(result.stdout.decode("cp1251"))
    log.write("\n")

Alphabet   = SymbolSet("абвгдежзийклмнопрстуфхцчшщьъюя0123456789 ,")
OneToNine  = SymbolSet("123456789")
Even       = SymbolSet("02468")
Odd        = SymbolSet("13579")
ZeroToNine = f"({Even}|{Odd})"
Epsilon = "((,)*-(,)+)"
ZeroToNineQ = f"({ZeroToNine}|{Epsilon})"
SuffQ = f"(( г,)|{Epsilon})"

Month29 = "(февруари)"
Month30 = "(април|юни|септември|ноември)"
Month31 = "(януари|март|май|юли|август|октомври|декември)"
Month   = f"({Month29}|{Month30}|{Month31})"

Date           = f"({OneToNine}|((1|2).{ZeroToNine})|(3.(0|1)))"
Year           = f"({OneToNine}.{ZeroToNineQ}.{ZeroToNineQ}.{ZeroToNineQ}.{ZeroToNineQ}.{ZeroToNineQ}.{ZeroToNineQ}.{ZeroToNineQ}.{ZeroToNineQ}.{ZeroToNineQ})"
DateExpression = f"({Date}. .{Month}. .{Year}.{SuffQ})"

all_ = f"({Alphabet}*)"

MaxDays30 = f"({all_}-({all_}.(30 ).{Month29}.{all_}))"
#t1 = open("test1.txt", "w")
#t1.write(MaxDays30)
#t1.close()
log.write("test1 result:\n")
stat(MaxDays30)

MaxDays31 = f"({all_}-({all_}.(31 ).({Month29}|{Month30}).{all_}))"
#t2 = open("test2.txt", "w")
#t2.write(MaxDays31)
#t2.close()
log.write("test2 result:\n")
stat(MaxDays31)

MaxDaysInMonth = f"({MaxDays30}&{MaxDays31})"
#t3 = open("test3.txt", "w")
#t3.write(MaxDaysInMonth)
#t3.close()
log.write("test3 result:\n")
stat(MaxDaysInMonth)

Div4    = f"(4|8|({ZeroToNine}*.(({Even}.{SymbolSet('048')})|({Odd}.{SymbolSet('26')}))))"
LeapYear = f"({Div4}-(({ZeroToNine}+-{Div4}).00))"
LeapDates = f"({all_}-((29 февруари ).({Year}-{LeapYear}).{SuffQ}))"
ValidDatesNoLeap = f"({DateExpression}&{MaxDaysInMonth})"
ValidDates    = f"({DateExpression}&{MaxDaysInMonth}&{LeapDates})"

t35 = open("test3,5.txt", "w")
t35.write(ValidDatesNoLeap)
t35.close()
log.write("test3.5 result (no leap years):\n")
stat(ValidDatesNoLeap)

t4 = open("test4.txt", "w")
t4.write(ValidDates)
t4.close()
log.write("test4 result:\n")
stat(ValidDates)

NonValidDates = f"({DateExpression}-{ValidDates})"
#t5 = open("test5.txt", "w")
#t5.write(NonValidDates)
#t5.close()
log.write("test5 result:\n")
stat(NonValidDates)
log.close()
