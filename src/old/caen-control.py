#!/usr/bin/env python3.6

import datetime
import enum
import os
import subprocess
import sys
import time
import tkinter as tk
import tkinter.ttk as ttk
from module import utility

#______________________________________________________________________________
class ELabel(enum.IntEnum):
  CATHODE = 0
  GEM = 1
  GATE = 2

#______________________________________________________________________________
class Controller(tk.Frame):
  caen_control = './bin/caen-control'
  host_name = '192.168.1.10'
  crate_type = 'SY5527'
  slot_number = 1
  max_channel = 6
  labels = ['Cathode', 'GEM', 'Gate']
  btext = ['Pw Off', 'Pw On']

  #____________________________________________________________________________
  def __init__(self):
    tk.Frame.__init__(self)
    self.master.title(f'CAENHV Controller for HypTPC (admin@{self.host_name})')
    self.master.geometry('800x400')
    self.master.resizable(0, 1)
    self.pack(fill=tk.Y, expand=True)
    self.power_status = []
    self.switch_functions = [self.switch_cathode,
                             self.switch_gem,
                             self.switch_gate]
    self.__make_menu()
    self.__make_labels()
    self.__make_buttons()
    self.__make_table()

  #____________________________________________________________________________
  def __make_buttons(self):
    font1 = ('Helvetica', -14)
    ftop = tk.Frame(self)
    ftop.pack(side=tk.TOP, fill=tk.X, padx=10, pady=10)
    fname = tk.Frame(ftop)
    fname.pack(side=tk.LEFT, padx=5)
    fbutton = tk.Frame(ftop)
    fbutton.pack(side=tk.LEFT, fill=tk.X, padx=10, pady=10)
    fv0set = tk.Frame(ftop)
    fv0set.pack(side=tk.LEFT, fill=tk.X, padx=10, pady=10)
    self.buttons = []
    self.v0set = []
    label = tk.Label(fv0set, font=font1, text='set')
    label.pack(side=tk.TOP, pady=5)
    for i, l in enumerate(self.labels):
      label = tk.Label(fname, font=font1, text=l)
      label.pack(side=tk.TOP, pady=5)
      self.power_status.append(False)
      self.buttons.append(tk.Button(fbutton, text=self.btext[0], font=font1,
                                    command=self.switch_functions[i-1]))
      self.buttons[i].pack(side=tk.TOP, padx=5)
      self.v0set.append(tk.Label(fv0set, font=font1, text='0'))
      self.v0set[i].pack(side=tk.TOP, pady=5)

  #____________________________________________________________________________
  def __make_labels(self):
    font = ('Helvetica', -14)
    self.label = tk.Label(self, bg='black', fg='blue', font=font,
                          text=f'CRATE={self.host_name}({self.crate_type})')
    # self.label.pack(side=tk.TOP, fill=tk.X)
    flabel1 = tk.Frame(self)
    flabel1.pack(side=tk.TOP, fill=tk.X, padx=10, pady=10)
    linfo = tk.Label(flabel1, font=font,
                     text=(f'CAEN {self.crate_type} '
                           + f'SLOT#{self.slot_number}  '
                           + f'Mod. A1526 6Ch Neg. 15KV 1mA  BdStatus='))
    linfo.pack(side=tk.LEFT, padx=5, pady=5)
    self.bdstatus = tk.Label(flabel1, font=font)
    self.bdstatus.pack(side=tk.LEFT)
    self.last_time = tk.Label(flabel1, text='Last update:', font=font)
    self.last_time.pack(side=tk.RIGHT, padx=5, pady=5)

  #____________________________________________________________________________
  def __make_menu(self):
    menubar = tk.Menu(self)
    self.master.config(menu=menubar)
    menu1 = tk.Menu(menubar, tearoff=0)
    self.lock_control = tk.BooleanVar()
    menu1.add_checkbutton(label='Lock control', onvalue=True, offvalue=False,
                          variable=self.lock_control)
    menu1.add_command(label='Quit', command=self.quit)
    menubar.add_cascade(label='Control', menu=menu1)

  #____________________________________________________________________________
  def __make_table(self):
    self.table = ttk.Treeview(self)
    self.table.place(x=10, y=100)
    # headers = self.get_caenhv().splitlines()[1].split()
    headers = ['CH', 'Name', 'V0Set', 'I0Set', 'VMon', 'IMon', 'RUp', 'RDWn',
               'Pw', 'Status']
    self.table['column'] = [i for i in range(len(headers))]
    self.table['show'] = 'headings'
    for i, h in enumerate(headers):
      self.table.column(i, width=80, anchor=tk.CENTER)
      self.table.heading(i, text=h)
    self.table.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

  #____________________________________________________________________________
  def get_caenhv(self):
    command = (f'{self.caen_control} {self.host_name} {self.crate_type} '
               + f'{self.slot_number} {self.max_channel} p')
    while True:
      ret = subprocess.run(command, shell=True,
                           stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE,
                           universal_newlines=True)
      if ret.returncode == 0:
        return ret.stdout
      else:
        utility.print_error(ret.stderr.rstrip())
        return ''
        # time.sleep(1)

  #____________________________________________________________________________
  def get_board_status(self, status):
    status = int(status)
    power_fail = (status >> 0) & 1
    firmware_error = (status >> 1) & 1
    calib_hv = (status >> 2) & 1
    calib_temp = (status >> 3) & 1
    under_temp = (status >> 4) & 1
    over_temp = (status >> 5) & 1
    if status == 0:
      return 'OK'
    elif power_fail == 1:
      return 'PowerFail'
    elif firmware_error == 1:
      return 'FirmwareError'
    elif calib_hv == 1:
      return 'CalibErrorHV'
    elif calib_temp == 1:
      return 'CalibErrorTemp'
    elif under_temp == 1:
      return 'UnderTemp'
    elif over_temp == 1:
      return 'OverTemp'
    else:
      return 'Unknown'

  #____________________________________________________________________________
  def get_channel_status(self, status):
    status = int(status)
    on = (status >> 0) & 1
    ramp_up = (status >> 1) & 1
    ramp_down = (status >> 2) & 1
    over_current = (status >> 3) & 1
    over_voltage = (status >> 4) & 1
    under_voltage = (status >> 5) & 1
    ext_trip = (status >> 6) & 1
    max_voltage = (status >> 7) & 1
    ext_disable = (status >> 8) & 1
    int_trip = (status >> 9) & 1
    unplugged = (status >> 10) & 1
    reserved = (status >> 11) & 1
    ov_prot = (status >> 12) & 1
    power_fail = (status >> 13) & 1
    temp_err = (status >> 14) & 1
    if status == 0 or status == 1:
      return ''
    elif ramp_up == 1:
      return 'RUp'
    elif ramp_down == 1:
      return 'RDown'
    elif over_current == 1:
      return 'OvCurr'
    elif over_voltage == 1:
      return 'OvVolt'
    elif under_voltage == 1:
      return 'UndVolt'
    elif ext_trip == 1:
      return 'ExtTrip'
    elif max_voltage == 1:
      return 'MaxV'
    elif ext_disable == 1:
      return 'ExtDis'
    elif int_trip == 1:
      return 'IntTrip'
    elif unplugged == 1:
      return 'Unplugged'
    # elif reserved == 1:
    #   return 'Reserved'
    elif ov_prot == 1:
      return 'OvProt'
    elif power_fail == 1:
      return 'PwFail'
    elif temp_err == 1:
      return 'TempErr'
    else:
      return 'Unknown'

  #____________________________________________________________________________
  def switch_cathode(self):
    status = not self.power_status[ELabel.CATHODE]
    self.power_status[ELabel.CATHODE] = status
    self.update_label()
    print('switch_cathode', status)

  #____________________________________________________________________________
  def switch_gate(self):
    status = not self.power_status[ELabel.GATE]
    self.power_status[ELabel.GATE] = status
    self.update_label()
    print('switch_gate', status)

  #____________________________________________________________________________
  def switch_gem(self):
    status = not self.power_status[ELabel.GEM]
    self.power_status[ELabel.GEM] = status
    self.update_label()
    print('switch_gem', status)

  #____________________________________________________________________________
  def update_caenhv(self):
    caenhv_lines = self.get_caenhv().splitlines()
    if len(caenhv_lines) == 0:
      return
    bdstatus = self.get_board_status(caenhv_lines[0].split()[1])
    self.bdstatus.config(text=f'{bdstatus}',
                         fg='black' if bdstatus == 'OK' else 'red')
    self.table.delete(*self.table.get_children())
    for i, c in enumerate(caenhv_lines):
      if i < 2:
        continue
      columns = c.split()
      columns[2] += ' V'
      columns[3] += ' uA'
      columns[4] += ' V'
      columns[5] += ' uA'
      columns[6] += ' V/s'
      columns[7] += ' V/s'
      columns[9] = self.get_channel_status(columns[9])
      self.table.insert('', 'end', tags=i, values=columns)
      if i%2 == 0:
        self.table.tag_configure(i, foreground='black', background='#eeeeee')
      else:
        self.table.tag_configure(i, foreground='black', background='white')
      if int(c.split()[9]) == 0:
        self.table.tag_configure(i, foreground='darkgray')
      elif int(c.split()[9]) > 7:
        self.table.tag_configure(i, background='yellow')
    self.table.config(selectmode='none')

  #____________________________________________________________________________
  def update_label(self):
    for i in ELabel:
      fgcolor = 'black' if self.power_status[i] else 'black'
      bgcolor = 'green' if self.power_status[i] else '#d9d9d9'
      self.buttons[i].config(text=self.btext[self.power_status[i]],
                             activeforeground=fgcolor, fg=fgcolor,
                             activebackground=bgcolor, bg=bgcolor,
                             state=(tk.DISABLED if self.lock_control.get() else
                                    tk.NORMAL))
    now = str(datetime.datetime.now())[:19]
    self.last_time.config(text=f'Last update: {now}')

  #____________________________________________________________________________
  def updater(self):
    self.update_caenhv()
    self.update_label()
    self.after(750, self.updater)

#______________________________________________________________________________
if __name__ == '__main__':
  app = Controller()
  app.updater()
  app.mainloop()
