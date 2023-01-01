import datetime
Import("env")

curr_date = datetime.datetime.now()

env.Append(CPPDEFINES=[
  ("ECC_VERSION_YEAR", curr_date.year),
  ("ECC_VERSION_MAJOR", curr_date.month),
  ("ECC_VERSION_MINOR", curr_date.day)
])
