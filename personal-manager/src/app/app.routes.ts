import { Routes } from '@angular/router';

import { HomeComponent } from './home/home.component';
import { CalendarComponent } from './calendar/calendar.component';
import { TasksComponent } from './tasks/tasks.component';
import { StickyNotesComponent } from './notes/sticky-notes.component';
import { MoneyManagerComponent } from './money-manager/money-manager.component';
import { CalculatorComponent } from './calculator/calculator.component';
import { AddressBookComponent } from './contacts/contacts.component';

export const routes: Routes = [
  { path: '', redirectTo: 'home', pathMatch: 'full' },
  { path: 'home', component: HomeComponent },
  { path: 'calendar', component: CalendarComponent },
  { path: 'tasks', component: TasksComponent },
  { path: 'notes', component: StickyNotesComponent },
  { path: 'checkbook', component: MoneyManagerComponent },
  { path: 'calculator', component: CalculatorComponent },
  { path: 'contacts', component: AddressBookComponent}
];
