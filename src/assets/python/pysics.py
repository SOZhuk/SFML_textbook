import pygame
import math
import random
pygame.init()

# all constants
# sRGB Luminance Y = 0.2126 R + 0.7152 G + 0.0722 B
MAX_VELOCITY = 10
DISPLAY_SIZE = [1200,800]
INSTRUCTION_AREA = 50
BORDER_WIDTH = 10
TAIL_LENGTH = 25
TOP_BORDER = 0
LEFT_BORDER = 0
RIGHT_BORDER = DISPLAY_SIZE[0]-2*BORDER_WIDTH
BOTTOM_BORDER = DISPLAY_SIZE[1]-2*BORDER_WIDTH

COLOR_WHITE = (255,255,255)
COLOR_BLUE = (0,0,255)
COLOR_YELLOW = (255,255,0)
COLOR_RED = (255,10,10)
COLOR_CYAN = (10,255,255)
COLOR_BLACK = (0,0,0)
COLOR_ORANGE = (255,180,0)
COLOR_GRAY = (128,128,128)
COLOR_GREEN = (10,255,10)

PLAYER_SIZE = (20,20)
R_COLORS = (COLOR_WHITE,COLOR_YELLOW,COLOR_CYAN,COLOR_BLUE,COLOR_ORANGE,(255,0,255))

CURVED = 0.000 #anti-clockwise in radians per tick
GRAVITY = 0.01
#air resistance
EPSILON = 0.00005
#bounce braking
BOUNCE_BRAKING=0.001


FONT_LIVES = pygame.font.SysFont("Courier New", 70,True)
SMB_HEART = '\u2665'

def randomcolour(luminance = 72):
    repeat = True
    while repeat:
        r = random.randint(0,255)
        b = random.randint(0,255)
        # sRGB Luminance Y = 0.2126 R + 0.7152 G + 0.0722 B
        # so G = (Y -0.2126R - 0.0722B) / 0.7152
        g = int(round((luminance - 0.2126*r - 0.0722*b)/0.7152))
        repeat = g>255 #if luminance too high for given random, generate again
    return (r,g,b)

class PowerBall:
    
    def __init__(self, parent, position=[0,0], speed=[0,0], size=20, color=[255,255,255], taillenth=10, bounce=True, changeColor=True):
        self.bounce_flag = bounce
        self.change_color = changeColor
        self.color = color
        self.parent = parent
        self.size = size
        self.position = list(position)
        self.speed = list(speed)
        self.sprite = pygame.Surface((size,size))
        self.tail = list()
        self.rect = self.sprite.get_rect()
        for i in range(taillenth):
            self.tail.append([self.sprite.copy(), self.position, self.color])
        self.rect.x, self.rect.y = self.position[0], self.position[1]
        return
    def set_velocity(self, vx, vy):
        self.speed[0] = vx
        self.speed[1] = vy
    
    def elastic_collide(self, other):
        # print ("v1 = ",self.speed)
        # print ("v2 = ",other.speed)
        v1x, v1y = self.speed[0],self.speed[1] #v1 velocity of this object
        v2x, v2y = other.speed[0], other.speed[1] #v2 velocity of colliding object
        # x1c, y1c = self.rect.center, (self.rect.top+self.rect.bottom)/2 #center of self
        # x2c, y2c = (other.rect.left+other.rect.right)/2, (other.rect.top+other.rect.bottom)/2 #cente of other
        ax,ay = other.rect.centerx-self.rect.centerx, other.rect.centery-self.rect.centery  # vector a between centers of objects
        r = math.sqrt(ax**2 + ay**2)
        if r<0.0001: return #dirty fix of rare occurence of division by zero
        
        ax, ay = ax/r,ay/r #normalize vector a to 1 length
        # print ("a=",ax,ay)
        v1px, v1py = ax*(v1x*ax+v1y*ay), ay*(v1x*ax+v1y*ay) #projection of v1 to a
        # print (v1px,v1py)
        v2px, v2py = ax*(v2x*ax+v2y*ay), ay*(v2x*ax+v2y*ay) #projection of v2 to a
        if (v1px*ax+v1py*ay<0) and (v2px*ax+v2py*ay<0):
            return #they are on the way from each other,
                   #so decide not to collide to evade weird artifacts of discrete calculus
        
        # print (v2px,v2py)
        # in elastic collision
        # we exchange momentum projections on vector between centers and keep orthogonal part
        # assuming we have same masses it's all the same with velocities
        self.speed[0], self.speed[1], other.speed[0], other.speed[1] = v1x-v1px+v2px, v1y-v1py+v2py, v2x-v2px+v1px, v2y-v2py+v1py
        self.set_velocity(v1x-v1px+v2px,v1y-v1py+v2py)
        other.set_velocity( v2x-v2px+v1px, v2y-v2py+v1py)
        # print ("HIT!")
        # print ("v1 = ",self.speed)
        # print ("v2 = ",other.speed)
        
        
        
    
    def colliderect(self, rect: pygame.Rect):
        return self.rect.colliderect(rect)
    
    def check_collision(self, other):
        return self.rect.colliderect(other.rect)
    
    
    def clear(self):
        self.sprite.set_colorkey(None)
        self.sprite.fill(COLOR_BLACK)
        self.parent.blit(self.sprite,self.rect)
        for t in self.tail[::-1]:
            t[0].set_colorkey(None)
            t[0].fill(COLOR_BLACK)
            self.parent.blit(t[0],t[1])
        return
    
    def draw(self):
        for t in self.tail[::-1]:
            t[0].set_colorkey(COLOR_BLACK)
            pygame.draw.circle(t[0], t[2], (self.size/2,self.size/2), self.size/2)    
            self.parent.blit(t[0],t[1])
        self.sprite.set_colorkey(COLOR_BLACK)
        pygame.draw.circle(self.sprite, self.color, (self.size/2,self.size/2), self.size/2)
        self.parent.blit(self.sprite,self.rect)
        return
    
    def move(self):
        #tail moving
        for i in range(1,len(self.tail))[::-1]:
            self.tail[i][2] = self.tail[i-1][2]
            self.tail[i][1] = self.tail[i-1][1]
            self.tail[i][2] = (self.tail[i-1][2][0]*0.9+self.tail[i-1][2][1]*0.1*random.random(),  
                        self.tail[i-1][2][1]*0.9+self.tail[i-1][2][2]*0.1*random.random(),
                        self.tail[i-1][2][2]*0.9+self.tail[i-1][2][0]*0.1*random.random())        
        if len(self.tail)>0:
            self.tail[0][2] = self.color
            self.tail[0][1] = self.position
        
        
        #curving
        self.rotate(CURVED)
        #air resistance
        self.speed = [self.speed[0]*(1-EPSILON),self.speed[1]*(1-EPSILON)]
        #gravity
        self.speed = [self.speed[0],self.speed[1]+GRAVITY]
        #moving
        self.position = [self.position[0]+self.speed[0],self.position[1]+self.speed[1]]
        (self.rect.x, self.rect.y) = (self.position[0], self.position[1])
        if self.bounce_flag:
            self.check_bounce()
        return

    def bounce(self): 
        self.speed = [self.speed[0]*(1-BOUNCE_BRAKING),self.speed[1]*(1-BOUNCE_BRAKING)]
        if self.change_color:
            game_display.fill(self.color)
            #self.color=random.choice(R_COLORS)
            self.color = randomcolour(100)
        return
    
    def escaped(self):
        return  (self.rect.top<-self.size or
                self.rect.left<-self.size or
                self.rect.bottom>self.parent.get_height()+self.size or
                self.rect.right>self.parent.get_width()+self.size)

    def check_bounce(self):
    #bounce check
        if self.rect.bottom>=self.parent.get_height() and self.speed[1]>0: 
            self.speed[1] *= -1
            # player_pos[1] = 2*(BOTTOM_BORDER-player_rect.height)-player_rect.y
            self.bounce()
            return True
        if self.rect.right>=self.parent.get_width() and self.speed[0]>0: 
            self.speed[0] *= -1
            self.bounce()
            return True
        if self.rect.top<=0 and self.speed[1]<0: 
            self.speed[1] *= -1
            self.bounce()
            return True
        if self.rect.left<=0 and self.speed[0]<0: 
            self.speed[0] *= -1
            self.bounce()
            return True
        return False
    def rotate(self, angle):
        self.speed = [self.speed[0]*math.cos(angle)+self.speed[1]*math.sin(angle),
                        self.speed[1]*math.cos(angle)-self.speed[0]*math.sin(angle)]
        return
    def accelerate(self, fraction):
        if fraction>0 and self.velocity()>MAX_VELOCITY:
            return
        self.speed = [self.speed[0]*fraction,self.speed[1]*fraction]
        return
    def velocity(self):
        return math.sqrt(self.speed[0]**2+self.speed[1]**2)

class Player(PowerBall):
    pass

def blitframe():
    game_display.blit(frame,(BORDER_WIDTH,BORDER_WIDTH+INSTRUCTION_AREA))


def print_instructions():
    fnt = pygame.font.SysFont("trebuchet ms", 26)
    text = fnt.render("left, right: change direction. up, down: accelerate and decelerate. Esc - exit",True,COLOR_WHITE,COLOR_BLUE)
    game_display.blit(text,[50,20])
    return

def create_enemy():
    return PowerBall(
                   frame,
                   (frame.get_width()-1, random.randint(20,frame.get_height()-20)),
                   [random.random()*3-5,random.random()*2-1],   #speed
                   50,COLOR_RED,2 #size, color, tail length
                   ,True,False
    )
def create_bonus():
    return PowerBall(
                   frame,
                   (random.randint(20,frame.get_width()-20), 1),
                   [random.random()*2-1,random.random()*2+1],   #speed
                   60,COLOR_GREEN,2 #size, color, tail length
                   ,True,False
    )
def create_nomad(spd=1):
    return PowerBall(
                   frame,
                   (random.randint(20,frame.get_width()-20), random.randint(20,frame.get_height()-20)), #position
                   [random.random()*spd*2-spd,random.random()*spd*2-spd],   #speed
                   20,randomcolour(200),0 #size, color, tail length
                   ,True,False
        )    

frametimer = pygame.time.Clock()

game_display = pygame.display.set_mode(DISPLAY_SIZE)
game_display.fill(COLOR_BLUE)

# starsky = pygame.image.load("st")

frame = pygame.Surface((DISPLAY_SIZE[0]-2*BORDER_WIDTH, DISPLAY_SIZE[1]-2*BORDER_WIDTH-INSTRUCTION_AREA))
frame.fill(COLOR_BLACK)


game_display.blit(frame,(BORDER_WIDTH, BORDER_WIDTH+INSTRUCTION_AREA))

player = PowerBall(
                   frame,
                   (random.randint(20,frame.get_width()-20), random.randint(20,frame.get_height()-20)), #position
                   [random.random()*10-5,random.random()*10-5],   #speed
                   20,COLOR_YELLOW,50 #size, color, tail length
                   ) 

nomads = list()
for i in range(1,2):
    nomads.append(create_nomad())
    
enemies = list()
bonuses = list()

player.clear()
frame.fill(COLOR_BLACK)
# game_display.blit(frame,(BORDER_WIDTH, BORDER_WIDTH))

CREATE_ENEMY = pygame.USEREVENT +1
CREATE_BONUS = pygame.USEREVENT +2
CREATE_NOMAD = pygame.USEREVENT +3
enemy_appear_delay = 2000
pygame.time.set_timer(CREATE_ENEMY,enemy_appear_delay)
pygame.time.set_timer(CREATE_BONUS,2500)
pygame.time.set_timer(CREATE_NOMAD,5000)

playing_flag = True
lives = 1
points = 0
level = 1

while playing_flag and lives>0:
    frametimer.tick(120)
    
    keys = pygame.key.get_pressed()
    
    if keys[pygame.constants.K_LEFT]:
        player.rotate(0.05)
    if keys[pygame.constants.K_RIGHT]:
        player.rotate(-0.05)
    if keys[pygame.constants.K_UP]:
        player.accelerate(1.03)
    if keys[pygame.constants.K_DOWN]:
        player.accelerate(0.9)
    if keys[pygame.constants.K_ESCAPE]:
            playing_flag = False
    
    for event in pygame.event.get():
        if event.type == pygame.constants.QUIT:
            playing_flag = False
        if event.type == CREATE_BONUS:
            if len(bonuses)<5:
                bonuses.append(create_bonus())
        if event.type == CREATE_ENEMY:
            if len(enemies)<5:
                enemies.append(create_enemy())
        if event.type == CREATE_NOMAD:
            if len(nomads)<100:
                nomads.append(create_nomad(level))
    
    frame.fill(COLOR_BLACK)
    player.move()
    player.draw()
    for p in nomads:
        p.move()
        p.draw()
        if p.escaped():
            nomads.remove(p)
    for p in enemies:
        p.move()
        p.draw()
        if p.escaped():
            enemies.remove(p)
    for p in bonuses:
        p.move()
        p.draw()
        if p.escaped():
            bonuses.remove(p)
    # check collisions
    for p in nomads:
        for obj in enemies+bonuses:
            if p.check_collision(obj):
                p.elastic_collide(obj)
        if player.check_collision(p):
            player.elastic_collide(p)
                
    for p in enemies:
        if p.check_collision(player):
            player.elastic_collide(p)
		
#            enemies.remove(p)
#            lives-=1
    for b in bonuses:
        if player.check_collision(b):
             player.elastic_collide(b)
#            bonuses.remove(b)
#            points+=1
#            if len(nomads)>0:
#                for i in range(len(nomads)//2):
#                    nomads.pop(0)
#            if points%5==0:
#                nomads.clear()
#                level+=1
#                enemy_appear_delay=round(enemy_appear_delay*0.66)
#                pygame.time.set_timer(CREATE_ENEMY,enemy_appear_delay)
#            if points%8==0:
#                lives+=1
            
    
    
    frame.blit(FONT_LIVES.render(SMB_HEART*lives,True,COLOR_RED),(10,10))
    blitframe()
    print_instructions()
    pygame.display.flip()

    if lives==0:
        txt = FONT_LIVES.render("G A M E   O V E R",True,COLOR_RED)
        frame.blit(txt,(frame.get_rect().centerx-txt.get_width()/2,frame.get_rect().centery-txt.get_height()/2))
        blitframe()
        pygame.display.flip()
        waiting = True
        while waiting:
            frametimer.tick(120)
            if keys[pygame.constants.K_ESCAPE]:
                waiting = False
            pygame.display.flip()
            
            
        

