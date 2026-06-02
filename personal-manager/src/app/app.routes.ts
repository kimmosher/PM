import { Routes } from '@angular/router';

import { HomeComponent } from './home/home.component';
import { CalendarComponent } from './calendar/calendar.component';
import { TasksComponent } from './tasks/tasks.component';
import { NotesComponent } from './notes/notes.component';
import { CheckbookComponent } from './checkbook/checkbook.component';
import { CalculatorComponent } from './calculator/calculator.component';
import { ContactsComponent } from './contacts/contacts.component';

export const routes: Routes = [
  { path: '', redirectTo: 'home', pathMatch: 'full' },
  { path: 'home', component: HomeComponent },
  { path: 'calendar', component: CalendarComponent },
  { path: 'tasks', component: TasksComponent },
  { path: 'notes', component: NotesComponent },
  { path: 'checkbook', component: CheckbookComponent },
  { path: 'calculator', component: CalculatorComponent },
  { path: 'contacts', component: ContactsComponent },
];
