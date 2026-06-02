import { Component } from '@angular/core';
import { NgOptimizedImage } from '@angular/common';
import { RouterLink } from '@angular/router';
@Component({
  selector: 'app-home',
  standalone: true,
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.css'],
  imports: [NgOptimizedImage, RouterLink]
})
export class HomeComponent {
   CalPath = 'assets/calendar.jpeg';
  CalcPath = 'assets/calculator.png';
  ChkPath = 'assets/checkbook.png';
  ContPath = 'assets/contacts.png';
  NotePath = 'assets/notepad.jpeg';
  TaskPath = 'assets/tasks.png';
  HomePath = 'assets/home.jpeg';
}