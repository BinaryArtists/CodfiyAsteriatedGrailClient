﻿#include "MaoXian.h"
enum CAUSE{
    QI_ZHA = 1201,
    MAO_XIAN_ZHE_TIAN_TANG = 1202,
    TOU_TIAN_HUAN_RI = 1203,
    TE_SHU_JIA_GONG = 1204,
    DI_XIA_FA_ZHE = 1205
};
enum SpecialActionId{
    SPECIAL_BUY,
    SPECIAL_SYNTHESIZE,
    SPECIAL_EXTRACT
};
MaoXian::MaoXian()
{
    makeConnection();
    setMyRole(this);
    connect(playerArea,SIGNAL(playerUnready()),this,SLOT(onUnready()));

    Button *qiZha,*touTianHuanRi,*teShuJiaGong;
    qiZha=new Button(3,QStringLiteral("欺诈"));
    buttonArea->addButton(qiZha);
    connect(qiZha,SIGNAL(buttonSelected(int)),this,SLOT(QiZha()));

    touTianHuanRi=new Button(4,QStringLiteral("偷天换日"));
    buttonArea->addButton(touTianHuanRi);
    connect(touTianHuanRi,SIGNAL(buttonSelected(int)),this,SLOT(TouTianHuanRi()));

    teShuJiaGong=new Button(5,QStringLiteral("特殊加工"));
    buttonArea->addButton(teShuJiaGong);
    connect(teShuJiaGong,SIGNAL(buttonSelected(int)),this,SLOT(TeShuJiaGong()));
}

void MaoXian::buy()
{
    Team*team=dataInterface->getMyTeam();

    int energy=team->getEnergy();

    state = DI_XIA_FA_ZHE;
    decisionArea->enable(0);
    decisionArea->enable(1);
    handArea->reset();
    playerArea->reset();
    tipArea->reset();

    if(energy<4)
        tipArea->setMsg(QStringLiteral("你摸3张牌，你方战绩区加两宝石"));
    if(energy==4)
        tipArea->setMsg(QStringLiteral("战绩区星石已有4个，购买只增加一宝石"));
    if(energy==5)
        tipArea->setMsg(QStringLiteral("战绩区星石数目已达上限，购买将不再增加星石"));
}

void MaoXian::extract()
{
    state = MAO_XIAN_ZHE_TIAN_TANG;
    tipArea->reset();
    handArea->reset();
    playerArea->reset();
    decisionArea->disable(0);
    decisionArea->enable(1);
    playerArea->enableMate(true);
    playerArea->setQuota(1);
    tipArea->setMsg(QStringLiteral("请先选择要给予的对象"));
}

void MaoXian::normal()
{
    Role::normal();
    Player* myself=dataInterface->getMyself();

    if(dataInterface->getMyTeam()->getEnergy()>0)
        buttonArea->enable(2);
//欺诈
    SafeList<Card*>handcards=dataInterface->getHandCards();
    for(int i=0;i<handcards.size()-1;i++)
        for(int j=i+1;j<handcards.size();j++)
            if(handcards[i]->getElement()==handcards[j]->getElement()){
                buttonArea->enable(3);
                break;
            }

//偷天换日 特殊加工
    if(myself->getEnergy()>0){
        if(dataInterface->getOtherTeam()->getGem()>0 && !onceUsed)
            buttonArea->enable(4);
        if(!onceUsed)
            buttonArea->enable(5);
    }
    unactionalCheck();
}

void MaoXian::attackOrMagic()
{
    Role::attackOrMagic();
    Player* myself=dataInterface->getMyself();
    SafeList<Card*>handcards=dataInterface->getHandCards();
    for(int i=0;i<handcards.size()-1;i++)
        for(int j=i+1;j<handcards.size();j++)
            if(handcards[i]->getElement()==handcards[j]->getElement()){
                buttonArea->enable(3);
                break;
            }
    if(myself->getEnergy()>0){
        if(dataInterface->getOtherTeam()->getGem()>0 && !onceUsed)
            buttonArea->enable(4);
        if(!onceUsed)
            buttonArea->enable(5);
    }
}

void MaoXian::attackAction()
{
    Role::attackAction();
    SafeList<Card*>handcards=dataInterface->getHandCards();
    for(int i=0;i<handcards.size()-1;i++)
        for(int j=i+1;j<handcards.size();j++)
            if(handcards[i]->getElement()==handcards[j]->getElement()){
                buttonArea->enable(3);
                break;
            }
}

void MaoXian::QiZha()
{
    //借用底层的攻击目标判定，直到onOkClicked
    state=10;
    qizha=true;
    handArea->reset();
    playerArea->reset();
    tipArea->reset();

    playerArea->setQuota(1);
    handArea->setQuota(2,3);

    decisionArea->enable(1);
    decisionArea->disable(0);

    handArea->enableAll();
    tipArea->setMsg(QStringLiteral("请选择欺诈的属性（3张同系自动视为暗灭）："));
    tipArea->addBoxItem(QStringLiteral("1.风"));
    tipArea->addBoxItem(QStringLiteral("2.水"));
    tipArea->addBoxItem(QStringLiteral("3.火"));
    tipArea->addBoxItem(QStringLiteral("4.地"));
    tipArea->addBoxItem(QStringLiteral("5.雷"));
    tipArea->showBox();
}

void MaoXian::TeShuJiaGong()
{
    state = TE_SHU_JIA_GONG;

    handArea->reset();
    playerArea->reset();
    tipArea->reset();

    decisionArea->enable(1);
    decisionArea->enable(0);
}

void MaoXian::TouTianHuanRi()
{
    state = TOU_TIAN_HUAN_RI;

    handArea->reset();
    playerArea->reset();
    tipArea->reset();

    decisionArea->enable(1);
    decisionArea->enable(0);
}

void MaoXian::cardAnalyse()
{
    Role::cardAnalyse();
    SafeList<Card*> selectedCards=handArea->getSelectedCards();
    try{
    switch (state)
    {
    case 10:
        foreach(Card*ptr,selectedCards)
            if(ptr->getElement()!=selectedCards[0]->getElement()){
                playerArea->reset();
                decisionArea->disable(0);
                break;
            }            
        break;
    }
    }catch(int error){
        logic->onError(error);
    }
}

void MaoXian::playerAnalyse()
{
    Player*dst;
    int gem;
    int crystal;
    switch (state)
    {
    case MAO_XIAN_ZHE_TIAN_TANG:
        dst=playerArea->getSelectedPlayers().at(0);
        gem=dataInterface->getMyTeam()->getGem();
        crystal=dataInterface->getMyTeam()->getCrystal();        
        tipArea->reset();
        tipArea->setMsg(QStringLiteral("请选择要提取的星石数："));
        switch(dst->getEnergyMax()-dst->getEnergy())
        {
        case 4:
        case 3:
        case 2:
            if(gem>=2)
                tipArea->addBoxItem(QStringLiteral("1.两个宝石"));
            if(crystal>=2)
                tipArea->addBoxItem(QStringLiteral("2.两个水晶"));
            if(gem>=1&&crystal>=1)
                tipArea->addBoxItem(QStringLiteral("3.一个宝石和一个水晶"));
        case 1:
            if(gem>=1)
                tipArea->addBoxItem(QStringLiteral("4.一个宝石"));
            if(crystal>=1)
                tipArea->addBoxItem(QStringLiteral("5.一个水晶"));
            decisionArea->enable(0);
            break;
        default:
            decisionArea->disable(0);
            break;
        }
        tipArea->showBox();        
        break;
    default:
        decisionArea->enable(0);
        break;
    }
}

void MaoXian::onOkClicked()
{
    if(qizha && state==10){
        state = QI_ZHA;
    }
    Role::onOkClicked();
    SafeList<Card*> selectedCards;
    SafeList<Player*>selectedPlayers;

    QString text;
    int flag;

    selectedCards=handArea->getSelectedCards();
    selectedPlayers=playerArea->getSelectedPlayers();

    network::Action* action;
    try{
    switch(state)
    {
//欺诈
    case QI_ZHA:
        action = newAction(ACTION_ATTACK_SKILL, QI_ZHA);
        if(selectedCards.size()==3){
            flag=6;
        }
        else{
            flag=tipArea->getBoxCurrentIndex()+1;
        }
        switch(flag)
        {
        case 1:
            action->add_args(66);
            break;
        case 2:
            action->add_args(133);
            break;
        case 3:
            action->add_args(87);
            break;
        case 4:
            action->add_args(45);
            break;
        case 5:
            action->add_args(110);
            break;
        case 6:
            action->add_args(39);
            break;
        }
        action->add_dst_ids(selectedPlayers[0]->getID());
        foreach(Card*ptr,selectedCards){
            action->add_card_ids(ptr->getID());
        }
        emit sendCommand(network::MSG_ACTION, action);
        qizha=false;
        gui->reset();
        break;
//特殊加工
    case TE_SHU_JIA_GONG:
        action = newAction(ACTION_MAGIC_SKILL, TE_SHU_JIA_GONG);
        emit sendCommand(network::MSG_ACTION, action);
        gui->reset();
        onceUsed = true;
        break;
//偷天换日
    case TOU_TIAN_HUAN_RI:
        action = newAction(ACTION_MAGIC_SKILL, TOU_TIAN_HUAN_RI);
        emit sendCommand(network::MSG_ACTION, action);
        gui->reset();
        onceUsed = true;
        break;
//冒险家天堂
    case MAO_XIAN_ZHE_TIAN_TANG:
        action = newAction(ACTION_SPECIAL_SKILL, MAO_XIAN_ZHE_TIAN_TANG);
        action->add_dst_ids(selectedPlayers[0]->getID());
        text = tipArea->getBoxCurrentText();
        switch(text[0].digitValue())
        {
        case 1:
            action->add_args(2);
            action->add_args(0);
            break;
        case 2:
            action->add_args(0);
            action->add_args(2);
            break;
        case 3:
            action->add_args(1);
            action->add_args(1);
            break;
        case 4:
            action->add_args(1);
            action->add_args(0);
            break;
        case 5:
            action->add_args(0);
            action->add_args(1);
            break;
        }
        emit sendCommand(network::MSG_ACTION, action);
        gui->reset();
        break;
//购买
    case DI_XIA_FA_ZHE:
        action = newAction(ACTION_SPECIAL, SPECIAL_BUY);
        int stone = dataInterface->getMyTeam()->getEnergy();
        if(stone<4)
        {
            action->add_args(2);
            action->add_args(0);
        }
        else if(stone==4)
        {
            action->add_args(1);
            action->add_args(0);
        }
        else
        {
            action->add_args(0);
            action->add_args(0);
        }
        gui->reset();
        emit sendCommand(network::MSG_ACTION, action);
        break;
    }
    }catch(int error){
        logic->onError(error);
    }
}

void MaoXian::onUnready()
{
    switch(state)
    {
    case MAO_XIAN_ZHE_TIAN_TANG:
        tipArea->reset();
        normal();
        break;
    }
}

void MaoXian::onCancelClicked()
{
    Role::onCancelClicked();
    switch(state)
    {
//欺诈
    case 10:
    case QI_ZHA:
//特殊加工
    case TE_SHU_JIA_GONG:
//偷天换日
    case TOU_TIAN_HUAN_RI:
//冒险者天堂
    case MAO_XIAN_ZHE_TIAN_TANG:
    case DI_XIA_FA_ZHE:
        if(actionFlag == 4){
            attackOrMagic();
        }
        else if(actionFlag == 0){
            normal();
        }
        qizha=false;
        break;
    }
}

void MaoXian::turnBegin()
{
    Role::turnBegin();
    qizha=false;
}

