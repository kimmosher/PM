import { Component } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { MatSidenavModule } from '@angular/material/sidenav';
import { NgOptimizedImage } from '@angular/common';
import { HomeComponent } from './home/home.component';

@Component({
  selector: 'app-root',
  standalone: true,
  templateUrl: './app.html',
  styleUrls: ['./app.css'],
  imports: [
    RouterOutlet,
    MatSidenavModule,
    NgOptimizedImage, 
    HomeComponent
  ]
})
export class AppComponent {
  CalPath = 'assets/calendar.jpeg';
  CalcPath = 'assets/calculator.png';
  ChkPath = 'assets/checkbook.png';
  ContPath = 'assets/contacts.png';
  NotePath = 'assets/notepad.jpeg';
  TaskPath = 'assets/tasks.png';
  HomePath = 'assets/home.jpeg';
}
